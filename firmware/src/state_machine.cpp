#include "state_machine.hpp"

#include <algorithm>
#include <cstdint>

#include "pdo_helper.hpp"

static constexpr uint8_t k_retry_count = 20;
static constexpr uint32_t k_timeout_period = 200;             // ms
static constexpr uint32_t k_state_transition_period = 1500;   // ms
static constexpr uint32_t k_big_step_period = 100;            // ms
static constexpr uint32_t k_blinking_period = 500;            // ms
static constexpr uint32_t k_double_click_period = 1000;       // ms
static constexpr uint32_t k_ui_refresh_period = 20;           // ms
static constexpr uint32_t k_fault_recovery_period = 1000;     // ms
static constexpr uint32_t k_sensor_update_period = 200;       // ms
static constexpr uint32_t k_ramp_up_period = 500;             // ms

static constexpr uint16_t k_big_step_size = 250;
static constexpr float k_low_voltage_threshold = 0.5F;
static constexpr uint8_t k_low_voltage_reading_count = 10;

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

static auto adjustAndClamp(uint16_t& value, uint16_t step, uint16_t min_val,
                           uint16_t max_val, int8_t multiplier) -> void {
    // 32-bit signed int is wide enough to prevent uint16_t underflow/overflow
    int32_t current = value;
    int32_t total_change = static_cast<int32_t>(step) * multiplier;
    int32_t result = current + total_change;
    // Clamp and cast back to uint16_t
    value = static_cast<uint16_t>(std::clamp(
        result, static_cast<int32_t>(min_val), static_cast<int32_t>(max_val)));
}

StateMachine::StateMachine(HardwareContext& hardware) : m_hw(hardware) {
    renderUI();
}

auto StateMachine::handleEvent(InitState& state, const SystemTickEvent& event)
    -> void {
    if (state.retry_count < k_retry_count) {
        state.elapsed_time += event.delta;
        if (state.elapsed_time >= k_timeout_period) {
            state.elapsed_time = 0;
            state.retry_count++;
            state.screen.updateProgress();
        }
    } else {
        // Reached max retries, load a default config and transition to main
        // state
        state.screen.setPdoProfileCount(0);
        state.transition_time += event.delta;
        if (state.transition_time >= k_state_transition_period) {
            m_configs.emplace_back(
                std::make_pair("", ConfigBuilder::buildDefault()));
            m_current_state =
                MainStateBuilder::buildFromConfig(m_configs.back().second);
            ;
        }
    }
    renderUI();
}

auto StateMachine::handleEvent(InitState&, const PdSinkStatusUpdateEvent& event)
    -> void {
    if (event.status.caps_received) {
        m_current_state = LoadingState{};
    }
}

auto StateMachine::handleEvent(LoadingState& state,
                               const SystemTickEvent& event) -> void {
    if (!state.are_pdos_loaded) {
        state.are_pdos_loaded = true;
        state.pdo_count = m_hw.pdsink.getPDSourcePowerCapabilities();
        for (auto i = 0; i < state.pdo_count; ++i) {
            IPdSink::Pdo pdo;
            if (m_hw.pdsink.getPdo(i, pdo) &&
                pdo.type != IPdSink::PdoType::AVS) {
                // Skip AVS PDOs, since the HW is not capable of handling them
                m_configs.emplace_back(std::make_pair(
                    pdoToString(pdo), ConfigBuilder::buildWithPdo(pdo)));
            }
        }
        state.screen.setPdoProfileCount(state.pdo_count);
    } else {
        state.transition_time += event.delta;
        if (state.transition_time >= k_state_transition_period) {
            if (state.pdo_count == 1) {
                auto next_state =
                    MainStateBuilder::buildFromConfig(m_configs.back().second);
                m_hw.pdsink.setPdoOutput(next_state.config.pdo.index,
                                         next_state.user_voltage,
                                         next_state.user_current);
                m_current_state = next_state;
            } else {
                m_current_state = MenuStateBuilder::build(m_configs, 0);
            }
        }
    }
    renderUI();
}

auto StateMachine::handleEvent(MenuState& state,
                               const RotaryEncoderEvent& event) -> void {
    // Handle encoder states
    if (event.encoder_state == RotaryEncoder::State::btn_short_press) {
        auto next_state = MainStateBuilder::MainStateBuilder::buildFromConfig(
            m_configs[state.screen.getSelectedMenuItem()].second);
        m_hw.pdsink.setPdoOutput(next_state.config.pdo.index,
                                 next_state.user_voltage,
                                 next_state.user_current);
        m_current_state = next_state;
    }
    // Update selected menu item based on encoder direction
    if (event.encoder_state == RotaryEncoder::State::rot_inc) {
        state.screen.selectNextMenuItem();
    } else if (event.encoder_state == RotaryEncoder::State::rot_dec) {
        state.screen.selectPreviousMenuItem();
    }
    renderUI();
}

auto StateMachine::handleEvent(MainState& state, const SystemTickEvent& event)
    -> void {
    state.updateStateTimers(event);
    // Handle blinking state for value editing mode
    if (state.is_editing && state.blinking_time >= k_blinking_period) {
        state.blinking_time = 0;
        state.blinking_state = !state.blinking_state;
    }
    // Determine visibility based on whether we are editing or static
    bool active_state = state.is_editing ? state.blinking_state : true;
    // Map selection to the final screen states
    bool highlight_voltage = (state.selection == Voltage) && active_state;
    bool highlight_current = (state.selection == Current) && active_state;
    // Update screen state based on selection and editing state
    state.screen.selectTargetVoltage(highlight_voltage)
        .selectTargetCurrent(highlight_current);
    state.handleFaultRecovery(m_hw);
    // Update screen with sensor data periodically
    if (state.sensor_update_time >= k_sensor_update_period) {
        state.sensor_update_time = 0;
        state.screen.setMeasuredVoltage(state.measured_voltage);
        state.screen.setMeasuredCurrent(state.measured_current);
        state.screen.setTemperature(state.measured_temperature);
    }
    state.handleShortCircuitDetection(m_hw);
    // Update UI periodically
    if (state.ui_refresh_time >= k_ui_refresh_period) {
        state.ui_refresh_time = 0;
        renderUI();
    }
}

auto StateMachine::handleEvent(MainState& state,
                               const RotaryEncoderEvent& event) -> void {
    switch (event.encoder_state) {
    case RotaryEncoder::State::btn_short_press:
        if (state.selection > None) {
            // if tv or tc is selected enter editing mode of the value
            if (state.is_editing) {
                m_hw.pdsink.setPdoOutput(state.config.pdo.index,
                                         state.user_voltage,
                                         state.user_current);
            }
            state.is_editing = !state.is_editing;
        } else {
            // when no item selected show menu on double click show menu
            // only if there is more than one config item and when the
            // output is turned off
            if (m_configs.size() <= 1 || state.output_enable) {
                break;
            }
            if (state.rotary_encoder_time <= k_double_click_period) {
                // Switch to menu state
                MenuScreen menu_screen;
                menu_screen.setTitle("Available PDOs");
                std::vector<std::string> profile_names;
                for (const auto& config : m_configs) {
                    profile_names.emplace_back(config.first);
                }
                menu_screen.setMenuItems(profile_names)
                    .selectMenuItem(state.config.pdo.index);
                m_current_state =
                    MenuStateBuilder::build(m_configs, state.config.pdo.index);
                renderUI();
                return;
            }
            state.rotary_encoder_time = 0;
        }
        break;
    case RotaryEncoder::State::btn_long_press:
        // ignore this while editing target voltage/current
        // or when a fault is detected
        if (state.is_editing || state.is_fault_detected) {
            break;
        }
        // toggle output enable
        state.setOutputEnable(m_hw, !state.output_enable);
        // reset ramp up timer to ignore checking for SCP for some time
        // while the output voltage is rising
        state.ramp_up_time = 0;
        break;
    case RotaryEncoder::State::rot_inc:
    case RotaryEncoder::State::rot_dec: {
        int8_t direction =
            (event.encoder_state == RotaryEncoder::State::rot_inc) ? 1 : -1;
        if (!state.config.is_editing_enabled) {
            break;
        }
        // select target voltage, target current or none
        if (state.is_editing) {
            bool big_step = state.rotary_encoder_time < k_big_step_period;
            state.rotary_encoder_time = 0;
            switch (state.selection) {
            case Voltage: {
                int8_t step_multiplier =
                    big_step ? k_big_step_size / state.config.pdo.voltage_step *
                                   direction
                             : direction;
                adjustAndClamp(state.user_voltage,
                               state.config.pdo.voltage_step,
                               state.config.pdo.voltage_min,
                               state.config.pdo.voltage_max, step_multiplier);
                break;
            }
            case Current: {
                int8_t step_multiplier =
                    big_step ? k_big_step_size / state.config.pdo.current_step *
                                   direction
                             : direction;
                adjustAndClamp(state.user_current,
                               state.config.pdo.current_step,
                               state.config.pdo.current_min,
                               state.config.pdo.current_max, step_multiplier);
                break;
            }
            default:
                break;
            }
            state.screen.setTargetVoltage(state.user_voltage)
                .setTargetCurrent(state.user_current);
        } else {
            if (direction > 0) {
                ++state.selection;
            } else {
                --state.selection;
            }
        }
        break;
    }
    case RotaryEncoder::State::rot_dec_while_btn_press:
    case RotaryEncoder::State::rot_inc_while_btn_press:
        // TODO implement me with constant current mode
        // implementation...
        break;
    case RotaryEncoder::State::idle:
    case RotaryEncoder::State::processed:
        // do nothing
        break;
    }
}

auto StateMachine::handleEvent(MainState& state, const SensorUpdateEvent& event)
    -> void {
    state.measured_voltage = event.voltage;
    state.measured_current = event.current;
    state.measured_temperature = event.temperature;
}

auto StateMachine::handleEvent(MainState& state,
                               const PdSinkStatusUpdateEvent& event) -> void {
    if (event.status.has_fault) {
        state.is_fault_detected = true;
        state.setOutputEnable(m_hw, false);
    }
}

auto StateMachine::handleEvent(MainState& state, const VoutStatusUpdateEvent&)
    -> void {
    state.setOutputEnable(m_hw, false);
}

auto StateMachine::MenuStateBuilder::build(
    const std::vector<std::pair<std::string, Config>>& configs,
    uint8_t selected_item) -> MenuState {
    MenuScreen menu_screen;
    menu_screen.setTitle("Available PDOs");
    std::vector<std::string> profile_names;
    for (const auto& config : configs) {
        profile_names.emplace_back(config.first);
    }
    menu_screen.setMenuItems(profile_names).selectMenuItem(selected_item);
    return MenuState{.screen = menu_screen};
}

auto StateMachine::MainStateBuilder::buildFromConfig(Config& config)
    -> MainState {
    auto main_state =
        StateMachine::MainState{.config = config,
                                .user_voltage = config.pdo.voltage_min,
                                .user_current = config.pdo.current_min};
    main_state.screen.setPdoType(config.pdo.type);
    main_state.screen.setTargetVoltage(main_state.user_voltage);
    main_state.screen.setTargetCurrent(main_state.user_current);
    return main_state;
}

auto StateMachine::MainState::updateStateTimers(const SystemTickEvent& event)
    -> void {
    blinking_time += event.delta;
    rotary_encoder_time += event.delta;
    ui_refresh_time += event.delta;
    fault_recovery_time += event.delta;
    ramp_up_time += event.delta;
    sensor_update_time += event.delta;
}

auto StateMachine::MainState::handleFaultRecovery(const HardwareContext& hw)
    -> void {
    if (is_fault_detected && fault_recovery_time >= k_fault_recovery_period) {
        fault_recovery_time = 0;
        if (!hw.pdsink.getStatus().has_fault) {
            // Fault is cleared, re negotiate the selected power profile
            is_fault_detected = false;
            hw.pdsink.setPdoOutput(config.pdo.index, user_voltage,
                                   user_current);
        }
    }
}

auto StateMachine::MainState::handleShortCircuitDetection(
    const HardwareContext& hw) -> void {
    // Handle when the LM73100 turns off the output due to a short circuit
    // Ramp up time can be high (couple hundered ms), it is used to update UI
    // and mark output as disabled. LM73100 immediately turns off output after
    // short circuit is detected.
    if (ramp_up_time >= k_ramp_up_period && output_enable &&
        measured_voltage < k_low_voltage_threshold) {
        ++low_voltage_reading_count;
        if (low_voltage_reading_count >= k_low_voltage_reading_count) {
            setOutputEnable(hw, false);
            low_voltage_reading_count = 0;
        }
    }
}

auto StateMachine::MainState::setOutputEnable(const HardwareContext& hw,
                                              bool enable) -> void {
    if (output_enable == enable) {
        return;
    }
    output_enable = enable;
    hw.output_enable.write(output_enable);
    screen.setOutputEnable(output_enable);
}

auto StateMachine::renderUI() -> void {
    auto& current_screen = std::visit(
        [](auto& state) -> Screen& { return state.screen; }, m_current_state);

    m_hw.oled.display(current_screen.build());
}
