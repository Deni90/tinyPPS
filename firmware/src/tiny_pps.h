#ifndef tiny_pps_h
#define tiny_pps_h

#include <string>
#include <vector>

#include "ap33772s.h"
#include "config.h"
#include "ina226.h"
#include "rotary_encoder.h"
#include "pico_i2c.h"
#include "pico_pin.h"
#include "ssd1306.h"

class TinyPPS {
  public:
    /**
     * Enumeration represending state machine states
     */
    enum class State { menu, main };

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
    Ina226 m_ina226;
    Ssd1306 m_oled;
    PicoPin m_rot_enc_a_pin, m_rot_enc_b_pin, m_rot_enc_btn_pin;
    RotaryEncoder m_rotary_encoder;
    Ap33772s m_usb_pd;
    State m_state;
    std::vector<std::pair<std::string, Config>> m_configs;
    unsigned int m_active_config_index;
    bool m_is_menu_enabled;
};

#endif   // tiny_pps_h