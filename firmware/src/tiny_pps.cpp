#include "tiny_pps.h"

#include <algorithm>
#include <cstdint>

#include "ap33772s.h"
#include "gpio_iface.h"
#include "loading_screen.h"
#include "main_screen.h"
#include "menu_screen.h"
#include "pdo_helper.h"
#include "pdsink_iface.h"
#include "rotary_encoder.h"

static constexpr uint k_rot_enc_btn_pin = 11;
static constexpr uint k_rot_enc_a_pin = 10;
static constexpr uint k_rot_enc_b_pin = 9;

static constexpr unsigned int k_i2c_sda_pin = 28;
static constexpr unsigned int k_i2c_scl_pin = 29;

static constexpr unsigned int k_pd_int_pin = 25;
static constexpr unsigned int k_output_enable_pin = 17;

static constexpr uint8_t k_ina226_addr = 0x40;

static constexpr unsigned int k_big_step_period = 75;         // ms
static constexpr unsigned int k_blinking_period = 500;        // ms
static constexpr unsigned int k_double_click_period = 1000;   // ms
static constexpr unsigned int k_measuring_period = 200;       // ms
static constexpr unsigned int k_fault_check_period = 1000;    // ms

static constexpr unsigned int k_i2c_speed = 400;   // kHz
static constexpr uint16_t k_ap33772s_vsel_min = 3300;

// https://product.tdk.com/system/files/dam/doc/product/sensor/ntc/chip-ntc-thermistor/data_sheet/datasheet_ntcgs103jx103dt8.pdf
// based on B value:
//                  [at 25/50C] 3380K typ.
//                  [at 25/85C] 3435K+-0.7%
static constexpr unsigned int k_ntc_tr25 = 10000;
static constexpr unsigned int k_ntc_tr50 = 4164;
static constexpr unsigned int k_ntc_tr75 = 1912;
static constexpr unsigned int k_ntc_tr100 = 987;

inline auto operator++(MainScreenSelection& selection) -> MainScreenSelection& {
    using T = std::underlying_type_t<MainScreenSelection>;
    selection = static_cast<MainScreenSelection>(
        (static_cast<T>(selection) + 1) %
        static_cast<T>(MainScreenSelection::Count));
    return selection;
}

inline auto operator--(MainScreenSelection& selection) -> MainScreenSelection& {
    int prev = (static_cast<int>(selection) - 1 +
                static_cast<int>(MainScreenSelection::Count)) %
               static_cast<int>(MainScreenSelection::Count);
    selection = static_cast<MainScreenSelection>(prev);
    return selection;
}

template <typename T>
static auto decrement_and_clamp(T& value, T step, T min_val, T max_val,
                                uint8_t multiplier = 1) -> void {
    value -= step * multiplier;
    value = std::clamp<T>(value, min_val, max_val);
}

template <typename T>
static auto increment_and_clamp(T& value, T step, T min_val, T max_val,
                                uint8_t multiplier = 1) -> void {
    value += step * multiplier;
    value = std::clamp<T>(value, min_val, max_val);
}

TinyPPS::TinyPPS()
    : m_ina226(&m_i2c, k_ina226_addr),
      m_oled(&m_i2c, Ssd1306::Type::ssd1306_128x64),
      m_rot_enc_a_pin(k_rot_enc_a_pin), m_rot_enc_b_pin(k_rot_enc_b_pin),
      m_rot_enc_btn_pin(k_rot_enc_btn_pin), m_pd_int(k_pd_int_pin),
      m_output_enable(k_output_enable_pin),
      m_rotary_encoder(&m_rot_enc_a_pin, &m_rot_enc_b_pin, &m_rot_enc_btn_pin),
      m_ap33772(&m_i2c), m_ap33772s(&m_i2c) {}

auto TinyPPS::initialize() -> bool {
    // Initialize a timer to repeat every 1 ms
    m_timer.start(
        1,
        [](void* ctx) {
            if (!ctx) {
                return;
            }
            auto self = static_cast<TinyPPS*>(ctx);
            self->m_system_time = self->m_system_time + 1;
        },
        this);

    m_pd_int.configure(IGpio::Direction::Input, IGpio::Pull::Down);
    m_pd_int.attachInterrupt(
        IGpio::Edge::Rising,
        [](IGpio& gpio, void* user) -> void {
            if (!user) {
                return;
            }
            auto* self = static_cast<TinyPPS*>(user);
            self->m_is_pd_interrupt_pending = true;
        },
        this);
    m_pd_int.enableInterrupt(true);

    m_output_enable.configure(IGpio::Direction::Output, IGpio::Pull::Down);

    m_rotary_encoder.initialize();
    m_i2c.initialize(i2c0, k_i2c_sda_pin, k_i2c_scl_pin, k_i2c_speed);
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

auto TinyPPS::handle() -> void {
    if (m_state == TinyPPS::State::init) {
        m_state = handleInitState();
    } else if (m_state == TinyPPS::State::menu) {
        m_state = handleMenuState();
    } else if (m_state == TinyPPS::State::main) {
        m_state = handleMainState();
    }
}

auto TinyPPS::handleInitState() -> TinyPPS::State {
    if (readPdos() == 0) {
        // If no profile present, create a default one
        m_configs.emplace_back(
            std::make_pair("", ConfigBuilder::buildDefault()));
    }
    // There is no need to show the menu if there is only one PDO available.
    // We can immediately switch to the main state
    if (m_configs.size() == 1) {

        return TinyPPS::State::main;
    }
    return TinyPPS::State::menu;
}

auto TinyPPS::handleMenuState() -> TinyPPS::State {
    State next_state = TinyPPS::State::menu;
    MenuScreen menu_screen(m_oled.getWidth(), m_oled.getHeight());
    std::vector<std::string> profile_names;
    // Fill in the menu with the available PDO profiles
    for (const auto& it : m_configs) {
        profile_names.emplace_back(it.first);
    }

    m_oled.display(menu_screen.setTitle("Available PDOs")
                       .setMenuItems(profile_names)
                       .selectMenuItem(m_menu_state_data.selected_menu_item)
                       .build());

    switch (m_rotary_encoder.getState()) {
    case RotaryEncoder::State::idle:
    case RotaryEncoder::State::processed:
        m_rotary_encoder.handle(m_system_time);
        break;
    case RotaryEncoder::State::btn_short_press:
        m_rotary_encoder.clearState();
        m_main_state_data.pdo_index = m_menu_state_data.selected_menu_item;
        next_state = State::main;
        break;
    case RotaryEncoder::State::rot_inc:
        m_rotary_encoder.clearState();
        if (m_menu_state_data.selected_menu_item <
            menu_screen.getMenuItems().size() - 1) {
            ++m_menu_state_data.selected_menu_item;
        } else {
            m_menu_state_data.selected_menu_item = 0;
        }
        break;
    case RotaryEncoder::State::rot_dec:
        m_rotary_encoder.clearState();
        if (m_menu_state_data.selected_menu_item == 0) {
            m_menu_state_data.selected_menu_item =
                menu_screen.getMenuItems().size() - 1;
        } else {
            --m_menu_state_data.selected_menu_item;
        }
        break;
    }
    return next_state;
}

auto TinyPPS::handleMainState() -> TinyPPS::State {
    Config& config = m_configs[m_main_state_data.pdo_index].second;

    if (!m_main_state_data.is_initialized) {
        m_main_state_data.is_initialized = true;
        // Start with min values for current and voltage
        m_main_state_data.target_voltage = config.pdo.voltage_min;
        m_main_state_data.target_current = config.pdo.current_min;
        // Request the min voltage and current for a selected PDO
        m_pd_sink->setPdoOutput(m_main_state_data.pdo_index,
                                m_main_state_data.target_voltage,
                                m_main_state_data.target_current);
    }
    m_main_state_data.screen.setPdoType(config.pdo.type)
        .setTargetVoltage(m_main_state_data.target_voltage)
        .setTargetCurrent(m_main_state_data.target_current)
        .setSupplyMode(config.supply_mode);

    switch (m_rotary_encoder.getState()) {
    case RotaryEncoder::State::idle:
    case RotaryEncoder::State::processed:
        m_rotary_encoder.handle(m_system_time);
        if ((m_system_time - m_main_state_data.sensor_reading_time_ms) >=
            k_measuring_period) {
            m_main_state_data.sensor_reading_time_ms = m_system_time;
            m_main_state_data.screen
                .setMeasuredVoltage(m_ina226.getBusVoltage())
                .setMeasuredCurrent(m_ina226.getCurrent())
                .setTemperature(m_pd_sink->getTemp());
        }
        // handle pending interrupt
        if (m_is_pd_interrupt_pending) {
            m_is_pd_interrupt_pending = false;
            // Check is the interrupt caused by some protection
            m_main_state_data.is_fault_detected =
                m_pd_sink->getStatus().has_fault;

            if (m_main_state_data.is_fault_detected) {
                m_main_state_data.fault_recovery_time_ms = m_system_time;
                // Disable output
                m_main_state_data.output_enable = false;
                enableOutput(m_main_state_data.output_enable);
                m_main_state_data.screen.setOutputEnable(
                    m_main_state_data.output_enable);
            }
        }
        // handle faults
        if (m_main_state_data.is_fault_detected) {
            // Periodically check if the fault is cleared
            if ((m_system_time - m_main_state_data.fault_recovery_time_ms) >=
                k_fault_check_period) {
                m_main_state_data.fault_recovery_time_ms = m_system_time;
                if (!m_pd_sink->getStatus().has_fault) {
                    // Fault is cleared, re negotiate the selected power
                    // profile
                    m_main_state_data.is_fault_detected = false;
                    m_pd_sink->setPdoOutput(m_main_state_data.pdo_index,
                                            m_main_state_data.target_voltage,
                                            m_main_state_data.target_current);
                }
            }
        }
        // handle blinking state for value editing mode
        if (m_main_state_data.is_editing &&
            (m_system_time - m_main_state_data.blinking_time_ms) >=
                k_blinking_period) {
            m_main_state_data.blinking_time_ms = m_system_time;
            m_main_state_data.blinking_state =
                !m_main_state_data.blinking_state;
        }
        {
            // Determine visibility based on whether we are editing or static
            bool active_state = m_main_state_data.is_editing
                                    ? m_main_state_data.blinking_state
                                    : true;
            // Map selection to the final screen states
            bool highlight_voltage =
                (m_main_state_data.selection == Voltage) && active_state;
            bool highlight_current =
                (m_main_state_data.selection == Current) && active_state;
            // Update screen state based on selection and editing state
            m_main_state_data.screen.selectTargetVoltage(highlight_voltage)
                .selectTargetCurrent(highlight_current);
        }
        break;
    case RotaryEncoder::State::btn_short_press:
        m_rotary_encoder.clearState();
        // handle short button press
        // if tv or tc is selected enter editing mode of the value
        if (m_main_state_data.selection > None) {
            if (!m_main_state_data.is_editing) {
                m_main_state_data.is_editing = true;
            } else {
                // TODO set a value
                m_pd_sink->setPdoOutput(m_main_state_data.pdo_index,
                                        m_main_state_data.target_voltage,
                                        m_main_state_data.target_current);
                m_main_state_data.is_editing = false;
            }
        } else {
            // when no item selected show menu on double click
            // show menu only if there is more than one config item and when the
            // output is turned off
            if (!m_configs.size() == 1 || m_main_state_data.output_enable) {
                break;
            }
            if ((m_system_time - m_main_state_data.rotary_encoder_time_ms) <=
                k_double_click_period) {
                // Switch to menu g_state
                m_rotary_encoder.clearState();
                m_main_state_data.is_initialized = false;
                return State::menu;
            }
            m_main_state_data.rotary_encoder_time_ms = m_system_time;
        }
        // if tv or tc is in editing mode set the value on button press
        break;
    case RotaryEncoder::State::btn_long_press:
        m_rotary_encoder.clearState();
        // handle long button press
        // ignore this while editing target voltage/current
        if (m_main_state_data.is_editing) {
            break;
        }
        if (!m_main_state_data.output_enable) {   // TODO check this
            m_main_state_data.selection = None;
        }
        // toggle the output enable only if no fault is detected
        if (!m_main_state_data.is_fault_detected) {
            m_main_state_data.output_enable = !m_main_state_data.output_enable;
            enableOutput(m_main_state_data.output_enable);
            m_main_state_data.screen.setOutputEnable(
                m_main_state_data.output_enable);
        }
        break;
    case RotaryEncoder::State::rot_dec:
        m_rotary_encoder.clearState();
        if (!config.is_editing_enabled) {
            break;
        }
        // handle rotary decrement
        // select target voltage, target current or none
        if (m_main_state_data.is_editing) {
            bool big_step = false;
            if ((m_system_time - m_main_state_data.rotary_encoder_time_ms) <=
                k_big_step_period) {
                big_step = true;
            }
            m_main_state_data.rotary_encoder_time_ms = m_system_time;
            switch (m_main_state_data.selection) {
            case Voltage:
                decrement_and_clamp(m_main_state_data.target_voltage,
                                    config.pdo.voltage_step,
                                    config.pdo.voltage_min,
                                    config.pdo.voltage_max, big_step ? 5 : 1);
                break;
            case Current:
                decrement_and_clamp(m_main_state_data.target_current,
                                    config.pdo.current_step,
                                    config.pdo.current_min,
                                    config.pdo.current_max, big_step ? 4 : 1);
                break;
            case None:
            case Count:
            default:
                break;
            }
        } else {
            --m_main_state_data.selection;
        }
        // decrement tv or tc in value editing mode
        break;
    case RotaryEncoder::State::rot_inc:
        m_rotary_encoder.clearState();
        if (!config.is_editing_enabled) {
            break;
        }
        // handle rotary increment
        // select target voltage, target current or none
        if (m_main_state_data.is_editing) {
            bool big_step = false;
            if ((m_system_time - m_main_state_data.rotary_encoder_time_ms) <=
                k_big_step_period) {
                big_step = true;
            }
            m_main_state_data.rotary_encoder_time_ms = m_system_time;
            switch (m_main_state_data.selection) {
            case Voltage:
                increment_and_clamp(m_main_state_data.target_voltage,
                                    config.pdo.voltage_step,
                                    config.pdo.voltage_min,
                                    config.pdo.voltage_max, big_step ? 5 : 1);
                break;
            case Current:
                increment_and_clamp(m_main_state_data.target_current,
                                    config.pdo.current_step,
                                    config.pdo.current_min,
                                    config.pdo.current_max, big_step ? 4 : 1);
                break;
            case None:
            case Count:
            default:
                break;
            }
        } else {
            ++m_main_state_data.selection;
        }
        break;
    case RotaryEncoder::State::rot_dec_while_btn_press:
    case RotaryEncoder::State::rot_inc_while_btn_press:
        m_rotary_encoder.clearState();
        // TODO implement me with constant current mode implementation...
        break;
    }

    m_oled.display(m_main_state_data.screen.build());
    return State::main;
}

auto TinyPPS::pdSinkInit() -> bool {
    if (m_ap33772.probe()) {
        m_pd_sink = &m_ap33772;
        m_ap33772.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        Ap33772::MaskReg mask;
        mask.ocp_en = 1;
        mask.otp_en = 1;
        mask.ovp_en = 1;
        m_ap33772.setMask(mask);
    } else if (m_ap33772s.probe()) {
        m_pd_sink = &m_ap33772s;
        m_ap33772s.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        m_ap33772s.setVselMin(k_ap33772s_vsel_min);
        Ap33772s::MaskReg mask;
        mask.ocp_msk = 1;
        mask.otp_msk = 1;
        mask.ovp_msk = 1;
        mask.uvp_msk = 1;
        m_ap33772s.setMask(mask);
    } else {
        return false;
    }

    enableOutput(false);

    return true;
}

auto TinyPPS::readPdos() -> int {
    int pdo_cnt = 0;
    LoadingScreen loading_screen(m_oled.getWidth(), m_oled.getHeight());
    m_oled.display(loading_screen.build());
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

auto TinyPPS::enableOutput(bool enable) -> void {
    m_output_enable.write(enable);
}
