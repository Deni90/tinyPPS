#ifndef state_machine_h
#define state_machine_h

#include <cstdint>
#include <string>
#include <vector>

#include "config.h"
#include "hardware.h"
#include "main_screen.h"
#include "menu_screen.h"

enum MainScreenSelection { None, Voltage, Current, Count };   // FIXME

class StateMachine {
  public:
    /**
     * Enumeration represending state machine states
     */
    enum class State { init, menu, main };

    /**
     * @brief Constructor
     * */
    StateMachine(HardwareContext& hardware);

    /**
     * @brief Initialize the module
     */
    auto initialize() -> void;

    /**
     * @brief Handle function used executing the main logic
     * @note Call this function from the main loop
     */
    auto handle() -> void;

  private:
    /**
     * @brief Data for the menu state
     */
    struct MenuStateData {
        MenuScreen screen;
        bool is_initialized{false};
        uint8_t selected_menu_item{0};
    };

    /**
     * @brief Data for the main state
     */
    struct MainStateData {
        MainScreen screen;
        MainScreenSelection selection{None};
        bool is_initialized{false};
        uint8_t pdo_index{0};
        uint16_t target_voltage{0};
        uint16_t target_current{0};
        bool is_fault_detected{false};
        bool output_enable{false};
        bool is_editing{false};
        bool blinking_state{false};
        uint32_t sensor_reading_time_ms{0};
        uint32_t rotary_encoder_time_ms{0};
        uint32_t fault_recovery_time_ms{0};
        uint32_t blinking_time_ms{0};
    };

    /**
     * @brief Function for handling the init state
     * @return Return the next state
     */
    auto handleInitState() -> State;

    /**
     * @brief Function for handling the menu state
     * @return Return the next state
     */
    auto handleMenuState(MenuStateData& data) -> State;

    /**
     * @brief Function for handling the main state
     * @return Return the next state
     */
    auto handleMainState(MainStateData& data) -> State;

    /**
     * @brief Sleep for a given number of milliseconds
     *
     * @param duration_ms The number of milliseconds to sleep
     */
    auto sleep_ms(uint32_t duration_ms) const -> void;

    HardwareContext& m_hw;
    State m_state{State::init};
    std::vector<std::pair<std::string, Config>> m_configs;
    volatile bool m_is_pd_interrupt_pending{false};
    volatile uint32_t m_system_time{0};
    MenuStateData m_menu_state_data;
    MainStateData m_main_state_data;
};

#endif   // state_machine_h
