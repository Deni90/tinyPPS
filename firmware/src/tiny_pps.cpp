#include "tiny_pps.h"

#include <algorithm>

#include "ap33772s.h"
#include "loading_screen.h"
#include "main_screen.h"
#include "menu_screen.h"
#include "pdo_helper.h"
#include "pdsink_iface.h"

static constexpr uint k_rot_enc_btn_pin = 11;
static constexpr uint k_rot_enc_a_pin = 10;
static constexpr uint k_rot_enc_b_pin = 9;

static constexpr unsigned int k_i2c_sda_pin = 28;
static constexpr unsigned int k_i2c_scl_pin = 29;

static constexpr unsigned int k_pd_int_pin = 25;

static constexpr uint8_t k_ina226_addr = 0x40;

static constexpr unsigned int k_big_step_period = 75;         // ms
static constexpr unsigned int k_blinking_period = 500;        // ms
static constexpr unsigned int k_double_click_period = 1000;   // ms
static constexpr unsigned int k_measuring_period = 200;       // ms
static constexpr unsigned int k_fault_check_period = 1000;    // ms

enum MainScreenSelection { None, Voltage, Current, Count };

inline MainScreenSelection& operator++(MainScreenSelection& s) {
    using T = std::underlying_type_t<MainScreenSelection>;
    s = static_cast<MainScreenSelection>(
        (static_cast<T>(s) + 1) % static_cast<T>(MainScreenSelection::Count));
    return s;
}

inline MainScreenSelection& operator--(MainScreenSelection& s) {
    int prev = (static_cast<int>(s) - 1 +
                static_cast<int>(MainScreenSelection::Count)) %
               static_cast<int>(MainScreenSelection::Count);
    s = static_cast<MainScreenSelection>(prev);
    return s;
}

TinyPPS::TinyPPS()
    : m_i2c(), m_ina226(&m_i2c, k_ina226_addr),
      m_oled(&m_i2c, Ssd1306::Type::ssd1306_128x64),
      m_rot_enc_a_pin(k_rot_enc_a_pin), m_rot_enc_b_pin(k_rot_enc_b_pin),
      m_rot_enc_btn_pin(k_rot_enc_btn_pin), m_pd_int(k_pd_int_pin),
      m_rotary_encoder(&m_rot_enc_a_pin, &m_rot_enc_b_pin, &m_rot_enc_btn_pin,
                       &m_debounce_clock),
      m_ap33772s(&m_i2c), m_pd_sink(nullptr), m_state(State::init),
      m_active_config_index(0), m_is_menu_enabled(false),
      m_is_pd_interrupt_pending(false), m_clock(0), m_debounce_clock(0),
      m_rotary_state_clock(0), m_measuring_clock(0), m_fault_clock(0) {}

bool TinyPPS::initialize() {
    // Initialize a timer to repeat every 1 ms
    m_timer.start(
        1,
        [](void* ctx) {
            if (!ctx) {
                return;
            }
            auto self = static_cast<TinyPPS*>(ctx);
            self->m_clock = self->m_clock + 1;
            self->m_debounce_clock = self->m_debounce_clock + 1;
            self->m_rotary_state_clock = self->m_rotary_state_clock + 1;
            self->m_measuring_clock = self->m_measuring_clock + 1;
            self->m_fault_clock = self->m_fault_clock + 1;
        },
        this);

    m_pd_int.configure(IGpio::Direction::Input, IGpio::Pull::Down);
    m_pd_int.attachInterrupt(
        IGpio::Edge::Rising,
        [](IGpio& gpio, void* user) {
            if (!user) {
                return;
            }
            auto self = static_cast<TinyPPS*>(user);
            self->m_is_pd_interrupt_pending = true;
        },
        this);
    m_pd_int.enableInterrupt(true);

    m_rotary_encoder.initialize();
    m_i2c.initialize(i2c0, k_i2c_sda_pin, k_i2c_scl_pin, 400);
    m_oled.initialize();
    if (!pdSinkInit()) {
        return false;
    }
    m_ina226.setAveragingMode(Ina226::AveragingMode::Samples128);
    if (!m_ina226.calibrate(0.01, 0.25)) {
        // Failed to calibrate INA226
        return false;
    }

    return true;
}

void TinyPPS::handle() {
    if (m_state == TinyPPS::State::init) {
        m_state = handleInitState();
    } else if (m_state == TinyPPS::State::menu) {
        m_state = handleMenuState();
    } else if (m_state == TinyPPS::State::main) {
        m_state = handleMainState();
    }
}

TinyPPS::State TinyPPS::handleInitState() {
    // There is no need to show the menu if there is none or one PDO available.
    // We can immediately switch to the main state
    if (readPdos() <= 1) {
        return TinyPPS::State::main;
    } else {
        m_is_menu_enabled = true;
        return TinyPPS::State::menu;
    }
}

TinyPPS::State TinyPPS::handleMenuState() {
    MenuScreen menu_screen(m_oled.getWidth(), m_oled.getHeight());
    std::vector<std::string> profile_names;
    for (const auto& it : m_configs) {
        profile_names.emplace_back(it.first);
    }
    static auto selected_menu_item = 0;
    menu_screen.setTitle("Available PDOs")
        .setMenuItems(profile_names)
        .selectMenuItem(selected_menu_item);
    while (true) {
        m_oled.display(menu_screen.build());
        while (m_rotary_encoder.getState() == RotaryEncoder::State::idle ||
               m_rotary_encoder.getState() == RotaryEncoder::State::processed) {
            m_rotary_encoder.Handle();
        }
        if (m_rotary_encoder.getState() ==
            RotaryEncoder::State::btn_short_press) {
            m_rotary_encoder.clearState();
            m_active_config_index = selected_menu_item;
            return State::main;
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_inc) {
            if (selected_menu_item < menu_screen.getMenuItems().size() - 1) {
                ++selected_menu_item;
            } else {
                selected_menu_item = 0;
            }
            menu_screen.selectMenuItem(selected_menu_item);
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_dec) {
            if (selected_menu_item == 0) {
                selected_menu_item = menu_screen.getMenuItems().size() - 1;
            } else {
                --selected_menu_item;
            }
            menu_screen.selectMenuItem(selected_menu_item);
        }
        m_rotary_encoder.clearState();
    }
    return State::menu;
}

TinyPPS::State TinyPPS::handleMainState() {
    if (!m_configs.size()) {
        // If no profile present, create a default one
        m_configs.emplace_back(
            std::make_pair("", ConfigBuilder::buildDefault()));
    }

    MainScreen main_screen(m_oled.getWidth(), m_oled.getHeight());
    Config& config = m_configs[m_active_config_index].second;

    MainScreenSelection selection = None;
    bool is_editing = false;
    bool blinking_state = true;
    bool output_enable = false;
    bool is_fault_detected = false;

    // Start with min values for current and voltage
    uint16_t target_voltage = config.pdo.voltage_min;
    uint16_t target_current = config.pdo.current_min;

    // Request the min voltage and current for a selected PDO
    m_pd_sink->setPdoOutput(m_active_config_index, target_voltage,
                            target_current);

    while (true) {
        switch (selection) {
        case None:
            main_screen.selectTargetVoltage(false).selectTargetCurrent(false);
            break;
        case Voltage:
            main_screen.selectTargetVoltage(true).selectTargetCurrent(false);
            break;
        case Current:
            main_screen.selectTargetVoltage(false).selectTargetCurrent(true);
            break;
        case Count:
        default:
            break;
        }
        main_screen.setPdoType(config.pdo.type)
            .setTargetVoltage(target_voltage)
            .setTargetCurrent(target_current)
            .setSupplyMode(config.supply_mode);
        m_oled.display(main_screen.build());
        while (m_rotary_encoder.getState() == RotaryEncoder::State::idle ||
               m_rotary_encoder.getState() == RotaryEncoder::State::processed) {
            m_rotary_encoder.Handle();

            // Handle pending interrupt
            if (m_is_pd_interrupt_pending) {
                m_is_pd_interrupt_pending = false;
                // Check is the interrupt caused by some protection
                is_fault_detected = m_pd_sink->getStatus().has_fault;
            }

            // handle value editing mode
            // the selected field should blink indicating the user the value
            // can be edited
            if (is_editing) {
                if (m_clock >= k_blinking_period) {
                    m_clock = 0;
                    blinking_state = !blinking_state;
                    switch (selection) {
                    case Voltage:
                        main_screen.selectTargetVoltage(blinking_state);
                        break;
                    case Current:
                        main_screen.selectTargetCurrent(blinking_state);
                        break;
                    case None:
                    case Count:
                    default:
                        break;
                    }
                    m_oled.display(main_screen.build());
                }
            }

            if (m_measuring_clock >= k_measuring_period) {
                m_measuring_clock = 0;
                main_screen.setMeasuredVoltage(m_ina226.getBusVoltage());
                main_screen.setMeasuredCurrent(m_ina226.getCurrent());
                main_screen.setTemperature(m_pd_sink->getTemp());
                m_oled.display(main_screen.build());
            }

            if (is_fault_detected) {
                // Disable output and update screen
                output_enable = false;
                m_pd_sink->enableOutput(output_enable);
                main_screen.setOutputEnable(output_enable);
                m_oled.display(main_screen.build());
                // Periodically check if the fault is cleared
                if (m_fault_clock >= k_fault_check_period) {
                    m_fault_clock = 0;
                    if (!m_pd_sink->getStatus().has_fault) {
                        // Fault is cleared, re negotiate the selected power
                        // profile
                        is_fault_detected = false;
                        m_pd_sink->setPdoOutput(m_active_config_index,
                                                target_voltage, target_current);
                    }
                }
            }
        }

        if (m_rotary_encoder.getState() ==
            RotaryEncoder::State::btn_short_press) {
            // handle short button press
            // if tv or tc is selected enter editing mode of the value
            if (selection > None) {
                if (!is_editing) {
                    is_editing = true;
                } else {
                    // TODO set a value
                    m_pd_sink->setPdoOutput(m_active_config_index,
                                            target_voltage, target_current);
                    is_editing = false;
                }
            } else {
                // when no item selected show menu on double click
                // show menu only if menu is enabled and when the output is
                // turned off
                if (!m_is_menu_enabled || output_enable) {
                    m_rotary_encoder.clearState();
                    continue;
                }
                if (m_rotary_state_clock <= k_double_click_period) {
                    // Switch to menu g_state
                    m_rotary_encoder.clearState();
                    return State::menu;
                }
                m_rotary_state_clock = 0;
            }
            // if tv or tc is in editing mode set the value on button press
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::btn_long_press) {
            // handle long button press
            // ignore this while editing target voltage/current
            if (is_editing) {
                m_rotary_encoder.clearState();
                continue;
            }
            if (!output_enable) {
                selection = None;
            }
            // toggle output only is no fault is detected
            if (!is_fault_detected) {
                output_enable = !output_enable;
                m_pd_sink->enableOutput(output_enable);
                main_screen.setOutputEnable(output_enable);
            }
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_dec) {
            if (!config.is_editing_enabled) {
                m_rotary_encoder.clearState();
                continue;
            }
            // handle rotary decrement
            // select target voltage, target current or none
            if (is_editing) {
                bool big_step = false;
                if (m_rotary_state_clock <= k_big_step_period) {
                    big_step = true;
                }
                m_rotary_state_clock = 0;
                switch (selection) {
                case Voltage:
                    target_voltage -= big_step ? config.pdo.voltage_step * 5
                                               : config.pdo.voltage_step;
                    target_voltage = std::clamp<uint16_t>(
                        target_voltage, config.pdo.voltage_min,
                        config.pdo.voltage_max);
                    break;
                case Current:
                    target_current -= big_step ? config.pdo.current_step * 4
                                               : config.pdo.current_step;
                    target_current = std::clamp<uint16_t>(
                        target_current, config.pdo.current_min,
                        config.pdo.current_max);
                    break;
                case None:
                case Count:
                default:
                    break;
                }
            } else {
                --selection;
            }
            // decrement tv or tc in value editing mode
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_inc) {
            if (!config.is_editing_enabled) {
                m_rotary_encoder.clearState();
                continue;
            }
            // handle rotary increment
            // select target voltage, target current or none
            if (is_editing) {
                bool big_step = false;
                if (m_rotary_state_clock <= k_big_step_period) {
                    big_step = true;
                }
                m_rotary_state_clock = 0;
                switch (selection) {
                case Voltage:
                    target_voltage += big_step ? 1000 : config.pdo.voltage_step;
                    target_voltage = std::clamp<uint16_t>(
                        target_voltage, config.pdo.voltage_min,
                        config.pdo.voltage_max);
                    break;
                case Current:
                    target_current += big_step ? config.pdo.current_step * 4
                                               : config.pdo.current_step;
                    target_current = std::clamp<uint16_t>(
                        target_current, config.pdo.current_min,
                        config.pdo.current_max);
                    break;
                case None:
                case Count:
                default:
                    break;
                }
            } else {
                ++selection;
            }
            // increment tv or tc in value editing mode
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_dec_while_btn_press) {
            if (!config.is_editing_enabled) {
                m_rotary_encoder.clearState();
                continue;
            }
            // handle rotary decrement while pressing a button
            // TODO implement constant current mode manualy for PPS/AVS PDOs
            // switch to constant voltage mode when the enable output is false
        } else if (m_rotary_encoder.getState() ==
                   RotaryEncoder::State::rot_inc_while_btn_press) {
            if (!config.is_editing_enabled) {
                m_rotary_encoder.clearState();
                continue;
            }
            // handle rotary increment while pressing a button
            // TODO implement constant current mode manualy for PPS/AVS PDOs
            // switch to constant current mode when the enable output is false
        }
        m_rotary_encoder.clearState();
    }
    return State::main;
}

bool TinyPPS::pdSinkInit() {
    if (m_ap33772s.probe()) {
        m_pd_sink = &m_ap33772s;
        // https://product.tdk.com/system/files/dam/doc/product/sensor/ntc/chip-ntc-thermistor/data_sheet/datasheet_ntcgs103jx103dt8.pdf
        // based on B value:
        //                  [at 25/50C] 3380K typ.
        //                  [at 25/85C] 3435K+-0.7%
        m_ap33772s.setNtc(10000, 4164, 1912, 987);
        m_ap33772s.setVselMin(3300);
        Ap33772s::MaskReg mask;
        mask.ocp_msk = 1;
        mask.otp_msk = 1;
        mask.ovp_msk = 1;
        mask.uvp_msk = 1;
        m_ap33772s.setMask(mask);
    } else {
        return false;
    }

    m_pd_sink->enableOutput(false);

    return true;
}

int TinyPPS::readPdos() {
    int pdo_cnt = 0;
    LoadingScreen loading_screen(m_oled.getWidth(), m_oled.getHeight());
    m_oled.display(loading_screen.build());
    m_clock = 0;
    // 1500ms should be enough to read PDOs
    for (int i = 0; i < 10; ++i) {
        sleep_ms(150);
        if (m_pd_sink->getStatus().caps_received) {
            sleep_ms(10);
            pdo_cnt = m_pd_sink->getPDSourcePowerCapabilities();
            // Fill in menu with PDOs
            for (uint8_t i = 0; i < Ap33772s::k_max_pdo_entries; ++i) {
                IPdSink::Pdo pdo;
                if (m_pd_sink->getPdo(i, pdo)) {
                    m_configs.emplace_back(std::make_pair(
                        pdoToString(pdo), ConfigBuilder::buildWithPdo(pdo)));
                }
            }
            sleep_ms(1000);
            break;
        }
        m_oled.display(loading_screen.updateProgress().build());
    }
    m_oled.display(loading_screen.setPdoProfileCount(pdo_cnt).build());
    sleep_ms(1500);

    return pdo_cnt;
}