#ifndef tiny_pps_h
#define tiny_pps_h

#include <string>
#include <vector>

#include "config.h"
#include "ina226.h"
#include "rotary_encoder.h"
#include "rp_i2c.h"
#include "rp_pin.h"
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

    RpI2c m_i2c;
    Ina226 m_ina226;
    Ssd1306 m_oled;
    RpPin m_rot_enc_a_pin, m_rot_enc_b_pin, m_rot_enc_btn_pin;
    RotaryEncoder m_rotary_encoder;
    State m_state;
    std::vector<std::pair<std::string, Config>> m_configs;
    unsigned int m_active_config_index;
};

#endif   // tiny_pps_h