#include "tiny_pps.h"

#include <algorithm>

#include "loading_screen.h"
#include "pdo_helper.h"

static constexpr unsigned int k_big_step_period = 75;         // ms
static constexpr unsigned int k_blinking_period = 500;        // ms
static constexpr unsigned int k_double_click_period = 1000;   // ms
static constexpr unsigned int k_measuring_period = 200;       // ms
static constexpr unsigned int k_fault_check_period = 1000;    // ms

static constexpr uint16_t k_big_step_size = 250;

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

TinyPPS::TinyPPS(HardwareContext& hardware) : m_hw(hardware) {}

auto TinyPPS::initialize() -> void {
    // Initialize a timer to repeat every 1 ms
    m_hw.timer.start(
        1,
        [](void* ctx) -> void {
            if (!ctx) {
                return;
            }
            auto* self = static_cast<TinyPPS*>(ctx);
            self->m_system_time = self->m_system_time + 1;
        },
        this);

    m_hw.pd_int.configure(IGpio::Direction::Input, IGpio::Pull::Down);
    m_hw.pd_int.attachInterrupt(
        IGpio::Edge::Rising,
        [](IGpio& gpio, void* user) -> void {
            if (!user) {
                return;
            }
            auto* self = static_cast<TinyPPS*>(user);
            self->m_is_pd_interrupt_pending = true;
        },
        this);
    m_hw.pd_int.enableInterrupt(true);
}

auto TinyPPS::handle() -> void {
    if (m_state == TinyPPS::State::init) {
        m_state = handleInitState();
    } else if (m_state == TinyPPS::State::menu) {
        auto tmp_state = handleMenuState(m_menu_state_data);
        if (tmp_state != m_state) {
            m_main_state_data.pdo_index = m_menu_state_data.selected_menu_item;
            m_state = tmp_state;
        }
    } else if (m_state == TinyPPS::State::main) {
        m_state = handleMainState(m_main_state_data);
    }
}

auto TinyPPS::handleInitState() -> TinyPPS::State {
    int pdo_cnt = 0;
    LoadingScreen loading_screen(m_hw.oled.getWidth(), m_hw.oled.getHeight());
    m_hw.oled.display(loading_screen.build());
    // 1500ms should be enough to read PDOs
    for (int i = 0; i < 10; ++i) {
        sleep_ms(150);
        if (m_hw.pdsink.getStatus().caps_received) {
            sleep_ms(10);
            pdo_cnt = m_hw.pdsink.getPDSourcePowerCapabilities();
            // Fill in menu with PDOs
            for (auto i = 0; i < pdo_cnt; ++i) {
                IPdSink::Pdo pdo;
                if (m_hw.pdsink.getPdo(i, pdo)) {
                    m_configs.emplace_back(std::make_pair(
                        pdoToString(pdo), ConfigBuilder::buildWithPdo(pdo)));
                }
            }
            sleep_ms(1000);
            break;
        }
        m_hw.oled.display(loading_screen.updateProgress().build());
    }
    m_hw.oled.display(loading_screen.setPdoProfileCount(pdo_cnt).build());
    sleep_ms(1500);

    if (pdo_cnt == 0) {
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

auto TinyPPS::handleMenuState(MenuStateData& data) -> TinyPPS::State {
    auto next_state = TinyPPS::State::menu;

    if (!data.is_initialized) {
        data.is_initialized = true;
        std::vector<std::string> profile_names;
        // Fill in the menu with the available PDO profiles
        for (const auto& it : m_configs) {
            profile_names.emplace_back(it.first);
        }
        data.screen.setTitle("Available PDOs").setMenuItems(profile_names);
    }

    const auto encoder_state = m_hw.encoder.getState();
    if (encoder_state != RotaryEncoder::State::idle &&
        encoder_state != RotaryEncoder::State::processed) {
        m_hw.encoder.clearState();
    }

    switch (encoder_state) {
    case RotaryEncoder::State::idle:
    case RotaryEncoder::State::processed:
        m_hw.encoder.handle(m_system_time);
        break;
    case RotaryEncoder::State::btn_short_press:
        next_state = State::main;
        break;
    case RotaryEncoder::State::rot_inc:
        if (data.selected_menu_item < data.screen.getMenuItems().size() - 1) {
            ++data.selected_menu_item;
        } else {
            data.selected_menu_item = 0;
        }
        break;
    case RotaryEncoder::State::rot_dec:
        if (data.selected_menu_item == 0) {
            data.selected_menu_item = data.screen.getMenuItems().size() - 1;
        } else {
            --data.selected_menu_item;
        }
        break;
    default:
        break;
    }
    m_hw.oled.display(
        data.screen.selectMenuItem(data.selected_menu_item).build());
    return next_state;
}

auto TinyPPS::handleMainState(MainStateData& data) -> TinyPPS::State {
    auto next_state = TinyPPS::State::main;
    Config& config = m_configs[data.pdo_index].second;

    if (!data.is_initialized) {
        data.is_initialized = true;
        // Start with min values for current and voltage
        data.target_voltage = config.pdo.voltage_min;
        data.target_current = config.pdo.current_min;
        // Request the min voltage and current for a selected PDO
        m_hw.pdsink.setPdoOutput(data.pdo_index, data.target_voltage,
                                 data.target_current);
        data.screen.setPdoType(config.pdo.type)
            .setTargetVoltage(data.target_voltage)
            .setTargetCurrent(data.target_current);
    }

    const auto encoder_state = m_hw.encoder.getState();
    if (encoder_state != RotaryEncoder::State::idle &&
        encoder_state != RotaryEncoder::State::processed) {
        m_hw.encoder.clearState();
    }

    if ((m_system_time - data.sensor_reading_time_ms) >= k_measuring_period) {
        data.sensor_reading_time_ms = m_system_time;
        data.screen.setMeasuredVoltage(m_hw.ina226.getBusVoltage())
            .setMeasuredCurrent(m_hw.ina226.getCurrent())
            .setTemperature(m_hw.pdsink.getTemp());
    }
    // handle pending interrupt
    if (m_is_pd_interrupt_pending) {
        m_is_pd_interrupt_pending = false;
        // Check is the interrupt caused by some protection
        data.is_fault_detected = m_hw.pdsink.getStatus().has_fault;

        if (data.is_fault_detected) {
            data.fault_recovery_time_ms = m_system_time;
            // Disable output
            data.output_enable = false;
            m_hw.output_enable.write(data.output_enable);
            data.screen.setOutputEnable(data.output_enable);
        }
    }
    // handle faults
    if (data.is_fault_detected) {
        // Periodically check if the fault is cleared
        if ((m_system_time - data.fault_recovery_time_ms) >=
            k_fault_check_period) {
            data.fault_recovery_time_ms = m_system_time;
            if (!m_hw.pdsink.getStatus().has_fault) {
                // Fault is cleared, re negotiate the selected power
                // profile
                data.is_fault_detected = false;
                m_hw.pdsink.setPdoOutput(data.pdo_index, data.target_voltage,
                                         data.target_current);
            }
        }
    }
    // handle blinking state for value editing mode
    if (data.is_editing &&
        (m_system_time - data.blinking_time_ms) >= k_blinking_period) {
        data.blinking_time_ms = m_system_time;
        data.blinking_state = !data.blinking_state;
    }
    // Determine visibility based on whether we are editing or static
    bool active_state = data.is_editing ? data.blinking_state : true;
    // Map selection to the final screen states
    bool highlight_voltage = (data.selection == Voltage) && active_state;
    bool highlight_current = (data.selection == Current) && active_state;
    // Update screen state based on selection and editing state
    data.screen.selectTargetVoltage(highlight_voltage)
        .selectTargetCurrent(highlight_current);

    switch (encoder_state) {
    case RotaryEncoder::State::idle:
    case RotaryEncoder::State::processed:
        m_hw.encoder.handle(m_system_time);
        break;
    case RotaryEncoder::State::btn_short_press:
        // handle short button press
        // if tv or tc is selected enter editing mode of the value
        if (data.selection > None) {
            if (!data.is_editing) {
                data.is_editing = true;
            } else {
                // TODO set a value
                m_hw.pdsink.setPdoOutput(data.pdo_index, data.target_voltage,
                                         data.target_current);
                data.is_editing = false;
            }
        } else {
            // when no item selected show menu on double click
            // show menu only if there is more than one config item and when the
            // output is turned off
            if (!m_configs.size() == 1 || data.output_enable) {
                break;
            }
            if ((m_system_time - data.rotary_encoder_time_ms) <=
                k_double_click_period) {
                // Switch to menu state
                data.is_initialized = false;
                next_state = State::menu;
            }
            data.rotary_encoder_time_ms = m_system_time;
        }
        // if tv or tc is in editing mode set the value on button press
        break;
    case RotaryEncoder::State::btn_long_press:
        // ignore this while editing target voltage/current
        // or when a fault is detected
        if (data.is_editing || data.is_fault_detected) {
            break;
        }
        {   // toggle output enable
            data.output_enable = !data.output_enable;
            m_hw.output_enable.write(data.output_enable);
            // read vout status (the output of the AND gate) and update screen
            // with it
            auto vout_status = m_hw.vout_status.read();
            data.screen.setOutputEnable(vout_status);
            // finally, write the vout status back to the output enable pin
            m_hw.output_enable.write(vout_status);
        }
        break;
    case RotaryEncoder::State::rot_dec:
        if (!config.is_editing_enabled) {
            break;
        }
        // handle rotary decrement
        // select target voltage, target current or none
        if (data.is_editing) {
            bool big_step = false;
            if ((m_system_time - data.rotary_encoder_time_ms) <=
                k_big_step_period) {
                big_step = true;
            }
            data.rotary_encoder_time_ms = m_system_time;
            switch (data.selection) {
            case Voltage:
                decrement_and_clamp(
                    data.target_voltage, config.pdo.voltage_step,
                    config.pdo.voltage_min, config.pdo.voltage_max,
                    big_step ? k_big_step_size / config.pdo.voltage_step : 1);
                break;
            case Current:
                decrement_and_clamp(
                    data.target_current, config.pdo.current_step,
                    config.pdo.current_min, config.pdo.current_max,
                    big_step ? k_big_step_size / config.pdo.current_step : 1);
                break;
            case None:
            case Count:
            default:
                break;
            }
            data.screen.setTargetVoltage(data.target_voltage)
                .setTargetCurrent(data.target_current);
        } else {
            --data.selection;
        }
        // decrement tv or tc in value editing mode
        break;
    case RotaryEncoder::State::rot_inc:
        if (!config.is_editing_enabled) {
            break;
        }
        // handle rotary increment
        // select target voltage, target current or none
        if (data.is_editing) {
            bool big_step = false;
            if ((m_system_time - data.rotary_encoder_time_ms) <=
                k_big_step_period) {
                big_step = true;
            }
            data.rotary_encoder_time_ms = m_system_time;
            switch (data.selection) {
            case Voltage:
                increment_and_clamp(
                    data.target_voltage, config.pdo.voltage_step,
                    config.pdo.voltage_min, config.pdo.voltage_max,
                    big_step ? k_big_step_size / config.pdo.voltage_step : 1);
                break;
            case Current:
                increment_and_clamp(
                    data.target_current, config.pdo.current_step,
                    config.pdo.current_min, config.pdo.current_max,
                    big_step ? k_big_step_size / config.pdo.current_step : 1);
                break;
            case None:
            case Count:
            default:
                break;
            }
            data.screen.setTargetVoltage(data.target_voltage)
                .setTargetCurrent(data.target_current);
        } else {
            ++data.selection;
        }
        break;
    case RotaryEncoder::State::rot_dec_while_btn_press:
    case RotaryEncoder::State::rot_inc_while_btn_press:
        // TODO implement me with constant current mode implementation...
        break;
    }

    m_hw.oled.display(data.screen.build());
    return next_state;
}

auto TinyPPS::sleep_ms(uint32_t duration_ms) const -> void {
    auto now = m_system_time;
    while (m_system_time - now < duration_ms) {
        // do nothing
    }
}
