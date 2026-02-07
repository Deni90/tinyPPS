#ifndef tiny_pps_h
#define tiny_pps_h

#include <string>
#include <vector>

#include "ap33772s.h"
#include "config.h"
#include "ina226.h"
#include "pico_gpio.h"
#include "pico_i2c.h"
#include "pico_timer.h"
#include "rotary_encoder.h"
#include "ssd1306.h"

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
    bool initialize();

    /**
     * @brief Handle function used executing the main logic
     * @note Call this function from the main loop
     */
    void handle();

  private:
    /**
     * @brief Function for handling the init state
     * @return Return the next state
     */
    State handleInitState();

    /**
     * @brief Function for handling the menu state
     * @return Return the next state
     */
    State handleMenuState();

    /**
     * @brief Function for handling the main state
     * @return Return the next state
     */
    State handleMainState();

    /**
     * @brief This function initializes the USB-PD controller
     */
    void usbPdInit();

    /**
     * @brief Handle PDO reading at startup
     *
     * @return The number of available PDOs
     */
    int readPdos();

    PicoI2c m_i2c;
    PicoGpio m_rot_enc_a_pin, m_rot_enc_b_pin, m_rot_enc_btn_pin;
    PicoGpio m_pd_int;
    PicoRepeatingTimer m_timer;
    Ina226 m_ina226;
    Ssd1306 m_oled;
    RotaryEncoder m_rotary_encoder;
    Ap33772s m_usb_pd;
    State m_state;
    std::vector<std::pair<std::string, Config>> m_configs;
    unsigned int m_active_config_index;
    bool m_is_menu_enabled;
    bool m_is_fault_detected;
    volatile uint32_t m_clock;
    volatile uint32_t m_debounce_clock;
    volatile uint32_t m_rotary_state_clock;
    volatile uint32_t m_measuring_clock;
    volatile uint32_t m_fault_clock;
};

#endif   // tiny_pps_h