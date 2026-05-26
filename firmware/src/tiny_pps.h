#ifndef tiny_pps_h
#define tiny_pps_h

#include <cstdint>
#include <string>
#include <vector>

#include "ap33772.h"
#include "ap33772s.h"
#include "config.h"
#include "ina226.h"
#include "main_screen.h"
#include "pdsink_iface.h"
#include "pico_gpio.h"
#include "pico_i2c.h"
#include "pico_timer.h"
#include "rotary_encoder.h"
#include "ssd1306.h"

enum MainScreenSelection { None, Voltage, Current, Count };   // FIXME

class TinyPPS {
  public:
    /**
     * Enumeration represending state machine states
     */
    enum class State { init, menu, main };

    /**
     * @brief Constructor
     * */
    TinyPPS();

    /**
     * @brief Initialize the module
     * @return True if the module is successfully initialized
     */
    auto initialize() -> bool;

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
        uint8_t selected_menu_item = 0;
    };

    /**
     * @brief Data for the main state
     */
    struct MainStateData {
        MainScreen screen{128, 64};   // FIXME
        MainScreenSelection selection = None;
        bool is_initialized = false;
        uint8_t pdo_index = 0;
        uint16_t target_voltage = 0;
        uint16_t target_current = 0;
        bool is_fault_detected = false;
        bool output_enable = false;
        bool is_editing = false;
        bool blinking_state = false;
        uint32_t sensor_reading_time_ms = 0;
        uint32_t rotary_encoder_time_ms = 0;
        uint32_t fault_recovery_time_ms = 0;
        uint32_t blinking_time_ms = 0;
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
    auto handleMenuState() -> State;

    /**
     * @brief Function for handling the main state
     * @return Return the next state
     */
    auto handleMainState() -> State;

    /**
     * @brief This function initializes the PD Sink controller
     @return true when the PD sink is successfully initialized
     @return false  when the PD sink is not initialized
     */
    auto pdSinkInit() -> bool;

    /**
     * @brief Handle PDO reading at startup
     *
     * @return The number of available PDOs
     */
    auto readPdos() -> int;

    /**
     * @brief Enable/Disable the output
     *
     * * @param enable Boolean value used to enable/disable the output
     */
    auto enableOutput(bool enable) -> void;

    PicoI2c m_i2c;
    PicoGpio m_rot_enc_a_pin, m_rot_enc_b_pin, m_rot_enc_btn_pin;
    PicoGpio m_pd_int;
    PicoGpio m_output_enable;
    PicoRepeatingTimer m_timer;
    Ina226 m_ina226;
    Ssd1306 m_oled;
    RotaryEncoder m_rotary_encoder;
    Ap33772 m_ap33772;
    Ap33772s m_ap33772s;
    IPdSink* m_pd_sink{nullptr};
    State m_state{State::init};
    std::vector<std::pair<std::string, Config>> m_configs;
    volatile bool m_is_pd_interrupt_pending{false};
    volatile uint32_t m_system_time{0};
    MenuStateData m_menu_state_data;
    MainStateData m_main_state_data;
};

#endif   // tiny_pps_h
