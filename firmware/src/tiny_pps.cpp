#include "tiny_pps.h"

#include "pico/stdlib.h"

#include <algorithm>
#include <stdio.h>

#include "loading_screen.h"
#include "main_screen.h"
#include "menu_screen.h"
#include "pdo_helper.h"

static constexpr uint k_rot_enc_btn_pin = 11;
static constexpr uint k_rot_enc_a_pin = 10;
static constexpr uint k_rot_enc_b_pin = 9;

static constexpr unsigned int k_i2c_sda_pin = 28;
static constexpr unsigned int k_i2c_scl_pin = 29;

static constexpr uint8_t k_ina226_addr = 0x40;

static constexpr unsigned int k_big_step_period = 75;         // ms
static constexpr unsigned int k_blinking_period = 500;        // ms
static constexpr unsigned int k_double_click_period = 1000;   // ms
static constexpr unsigned int k_measuring_period = 200;       // ms

static volatile uint32_t g_clock = 0;
static volatile uint32_t g_debounce_clock = 0;
static volatile uint32_t g_rotary_state_clock = 0;
static volatile uint32_t g_measuring_clock = 0;

TinyPPS::TinyPPS()
    : m_i2c(), m_ina226(&m_i2c, k_ina226_addr),
      m_oled(&m_i2c, Ssd1306::Type::ssd1306_128x64),
      m_rot_enc_a_pin(k_rot_enc_a_pin), m_rot_enc_b_pin(k_rot_enc_b_pin),
      m_rot_enc_btn_pin(k_rot_enc_btn_pin),
      m_rotary_encoder(&m_rot_enc_a_pin, &m_rot_enc_b_pin, &m_rot_enc_btn_pin,
                       &g_debounce_clock),
      m_usb_pd(&m_i2c), m_state(State::menu), m_active_config_index(0),
      m_is_menu_enabled(false) {}

bool TinyPPS::initialize() {
    stdio_init_all();
    // Initialize a timer to repeat every 1 ms
    static struct repeating_timer timer;
    add_repeating_timer_ms(
        1,
        [](__unused struct repeating_timer* t) -> bool {
            g_clock = g_clock + 1;
            g_debounce_clock = g_debounce_clock + 1;
            g_rotary_state_clock = g_rotary_state_clock + 1;
            g_measuring_clock = g_measuring_clock + 1;
            return true;
        },
        NULL, &timer);

    m_rotary_encoder.initialize();
    m_i2c.initialize(i2c0, k_i2c_sda_pin, k_i2c_scl_pin, 400);
    usbPdInit();
    m_oled.initialize();
    m_ina226.setAveragingMode(Ina226::AveragingMode::Samples128);
    if (!m_ina226.calibrate(0.01, 0.25)) {
        // Failed to calibrate INA226
        return false;
    }

    // There is no need to show the menu if there is none or one PDO available.
    // We can immediately switch to the main state
    if (readPdos() <= 1) {
        m_state = TinyPPS::State::main;
    } else {
        m_state = TinyPPS::State::menu;
        m_is_menu_enabled = true;
    }

    return true;
}

void TinyPPS::handle() {
    if (m_state == TinyPPS::State::menu) {
        m_state = handleMenuState();
    } else if (m_state == TinyPPS::State::main) {
        m_state = handleMainState();
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

    int8_t selection = 0;
    bool is_editing = false;
    bool blinking_state = true;
    bool output_enable = false;

    // Start with min values for current and voltage
    uint16_t target_voltage = config.pdo.voltage_min;
    uint16_t target_current = config.pdo.current_min;

    // Request the min voltage and current for a selected PDO
    m_usb_pd.setPdoOutput(m_active_config_index, target_voltage,
                          target_current);

    while (true) {
        switch (selection) {
        case 0:
            main_screen.selectTargetVoltage(false).selectTargetCurrent(false);
            break;
        case 1:
            main_screen.selectTargetVoltage(true).selectTargetCurrent(false);
            break;
        case 2:
            main_screen.selectTargetVoltage(false).selectTargetCurrent(true);
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
            // handle value editing mode
            // the selected field should blink indicating the user the value
            // can be edited
            if (is_editing) {
                if (g_clock >= k_blinking_period) {
                    g_clock = 0;
                    blinking_state = !blinking_state;
                    switch (selection) {
                    case 1:
                        main_screen.selectTargetVoltage(blinking_state);
                        break;
                    case 2:
                        main_screen.selectTargetCurrent(blinking_state);
                        break;
                    }
                    m_oled.display(main_screen.build());
                }
            }

            if (g_measuring_clock >= k_measuring_period) {
                g_measuring_clock = 0;
                main_screen.setMeasuredVoltage(m_ina226.getBusVoltage());
                main_screen.setMeasuredCurrent(m_ina226.getCurrent());
                main_screen.setTemperature(m_usb_pd.getTemp());
                m_oled.display(main_screen.build());
            }
        }

        if (m_rotary_encoder.getState() ==
            RotaryEncoder::State::btn_short_press) {
            // handle short button press
            // if tv or tc is selected enter editing mode of the value
            if (selection > 0) {
                if (!is_editing) {
                    is_editing = true;
                } else {
                    // TODO set a value
                    m_usb_pd.setPdoOutput(m_active_config_index, target_voltage,
                                          target_current);
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
                if (g_rotary_state_clock <= k_double_click_period) {
                    // Switch to menu g_state
                    m_rotary_encoder.clearState();
                    return State::menu;
                }
                g_rotary_state_clock = 0;
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
            // toggle output enable if all conditions are met
            if (!output_enable) {
                selection = 0;
            }
            output_enable = !output_enable;
            m_usb_pd.enableOutput(output_enable);
            main_screen.setOutputEnable(output_enable);
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
                if (g_rotary_state_clock <= k_big_step_period) {
                    big_step = true;
                }
                g_rotary_state_clock = 0;
                switch (selection) {
                case 1:
                    target_voltage -= big_step ? config.pdo.voltage_step * 5
                                               : config.pdo.voltage_step;
                    target_voltage = std::clamp<uint16_t>(
                        target_voltage, config.pdo.voltage_min,
                        config.pdo.voltage_max);
                    break;
                case 2:
                    target_current -= big_step ? config.pdo.current_step * 4
                                               : config.pdo.current_step;
                    target_current = std::clamp<uint16_t>(
                        target_current, config.pdo.current_min,
                        config.pdo.current_max);
                    break;
                }
            } else {
                if (selection) {
                    --selection;
                } else {
                    selection = 2;
                }
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
                if (g_rotary_state_clock <= k_big_step_period) {
                    big_step = true;
                }
                g_rotary_state_clock = 0;
                switch (selection) {
                case 1:
                    target_voltage += big_step ? 1000 : config.pdo.voltage_step;
                    target_voltage = std::clamp<uint16_t>(
                        target_voltage, config.pdo.voltage_min,
                        config.pdo.voltage_max);
                    break;
                case 2:
                    target_current += big_step ? config.pdo.current_step * 4
                                               : config.pdo.current_step;
                    target_current = std::clamp<uint16_t>(
                        target_current, config.pdo.current_min,
                        config.pdo.current_max);
                    break;
                }
            } else {
                if (selection < 2) {
                    ++selection;
                } else {
                    selection = 0;
                }
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

void TinyPPS::usbPdInit() {
    m_usb_pd.enableOutput(false);
    // https://product.tdk.com/system/files/dam/doc/product/sensor/ntc/chip-ntc-thermistor/data_sheet/datasheet_ntcgs103jx103dt8.pdf
    // based on B value:
    //                  [at 25/50C] 3380K typ.
    //                  [at 25/85C] 3435K+-0.7%
    m_usb_pd.setNtc(10000, 4164, 1912, 987);
    m_usb_pd.setVselMin(3300);
}

int TinyPPS::readPdos() {
    int pdo_cnt = 0;
    LoadingScreen loading_screen(m_oled.getWidth(), m_oled.getHeight());
    m_oled.display(loading_screen.build());
    g_clock = 0;
    // 1500ms should be enough to read PDOs
    for (int i = 0; i < 10; ++i) {
        sleep_ms(150);
        if (m_usb_pd.isNewPdoAvailable()) {
            sleep_ms(10);
            pdo_cnt = m_usb_pd.getPDSourcePowerCapabilities();
            // Fill in menu with PDOs
            for (uint8_t i = 0; i < Ap33772s::k_max_pdo_entries; ++i) {
                Ap33772s::Pdo pdo;
                if (m_usb_pd.getPdo(i, pdo)) {
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