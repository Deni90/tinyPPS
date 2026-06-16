#ifndef state_machine_hpp
#define state_machine_hpp

#include <cstdint>
#include <string>
#include <variant>

#include "config.hpp"
#include "event.hpp"
#include "loading_screen.hpp"
#include "main_screen.hpp"
#include "menu_screen.hpp"

enum MainScreenSelection { None, Voltage, Current, Count };   // FIXME

class StateMachine {
  public:
    /**
     * @brief Constructor
     * @param hardware The hardware context
     */
    StateMachine(HardwareContext& hardware);

    /**
     * @brief Dispatch an event to the current state
     * @param event The event to dispatch
     */
    auto dispatch(const SystemEvent& event) -> void {
        std::visit([this](auto& state,
                          const auto& evt) -> auto { handleEvent(state, evt); },
                   m_current_state, event);
    }

  private:
    struct InitState {
        LoadingScreen screen;
        uint8_t retry_count{0};
        uint32_t elapsed_time{0};
        uint32_t transition_time{0};
    };

    struct LoadingState {
        LoadingScreen screen;
        bool are_pdos_loaded{false};
        uint8_t pdo_count{0};
        uint32_t transition_time{0};
    };

    struct MenuState {
        MenuScreen screen;
    };

    struct MenuStateBuilder {
        static auto
        build(const std::vector<std::pair<std::string, Config>>& configs,
              uint8_t selected_item) -> MenuState;
    };

    struct MainState {
        Config config;
        MainScreen screen{};
        bool is_editing{false};
        uint32_t blinking_time{0};
        bool blinking_state{false};
        MainScreenSelection selection{None};
        bool output_enable{false};
        uint32_t rotary_encoder_time{0};
        uint32_t ui_refresh_time{0};
        uint16_t user_voltage{0};
        uint16_t user_current{0};
        bool is_fault_detected{false};
        uint32_t fault_recovery_time{0};
        float measured_voltage{0.0F};
        float measured_current{0.0F};
        uint8_t measured_temperature{0};
        uint32_t sensor_update_time{0};
        uint32_t ramp_up_time{0};
        uint32_t low_voltage_reading_count{0};

        auto updateStateTimers(const SystemTickEvent& event) -> void;
        auto handleFaultRecovery(const HardwareContext& hw) -> void;
        auto handleShortCircuitDetection(const HardwareContext& hw) -> void;
        auto setOutputEnable(const HardwareContext& hw, bool enable) -> void;
    };

    struct MainStateBuilder {
        static auto buildFromConfig(Config& config) -> MainState;
    };

    using State = std::variant<InitState, LoadingState, MenuState, MainState>;

    auto handleEvent(InitState& state, const SystemTickEvent& event) -> void;
    auto handleEvent(InitState& state, const PdSinkStatusUpdateEvent& event)
        -> void;

    auto handleEvent(LoadingState& state, const SystemTickEvent& event) -> void;

    auto handleEvent(MenuState& state, const RotaryEncoderEvent& event) -> void;

    auto handleEvent(MainState& state, const SystemTickEvent& event) -> void;
    auto handleEvent(MainState& state, const RotaryEncoderEvent& event) -> void;
    auto handleEvent(MainState& state, const SensorUpdateEvent& event) -> void;
    auto handleEvent(MainState& state, const PdSinkStatusUpdateEvent& event)
        -> void;
    auto handleEvent(MainState& state, const VoutStatusUpdateEvent& event)
        -> void;

    template <typename S, typename E>
    auto handleEvent(S&, const E&) -> void {}

    auto renderUI() -> void;

    HardwareContext& m_hw;
    State m_current_state{InitState{}};
    std::vector<std::pair<std::string, Config>> m_configs;
};

#endif   // state_machine_hpp
