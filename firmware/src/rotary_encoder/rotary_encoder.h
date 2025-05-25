#ifndef rotary_encoder_h
#define rotary_encoder_h

#include <cstdint>

#include "pin_iface.h"

class RotaryEncoder {
  public:
    /**
     * @brief Enumeration describing rotary encoder states
     */
    enum class State {
        idle,
        processed,
        rot_inc,
        rot_dec,
        rot_inc_while_btn_press,
        rot_dec_while_btn_press,
        btn_short_press,
        btn_long_press
    };

    /**
     * @brief Constructor
     *
     * Initialize rotary encoder with its pins and a clock used for debouncing
     *
     * @param[in] a_gpio A pin of the rotary encoder
     * @param[in] b_gpio B pin of the rotary encoder
     * @param[in] btn_gpio Button pin of the rotary encoder
     * @param[in] clock Pointer to a variable that is used for debouncing
     */
    RotaryEncoder(IPin* a_gpio, IPin* b_gpio, IPin* btn_gpio,
                  volatile uint32_t* clock);

    /**
     * @brief Handle function for the rotary encoder.
     *
     * Call this function in a loop.
     */
    void Handle();

    /**
     * @brief Get state of the rotsry encoder
     *
     * @return state
     */
    State getState() const { return m_state; }

    /**
     * @brief Clear state of the rotary encoder indicating that the state is
     * processed.
     */
    void clearState() { m_state = State::processed; }

  private:
    State m_state;
    volatile uint32_t* m_clock;
    IPin* m_a_gpio;
    IPin* m_b_gpio;
    IPin* m_btn_gpio;
};

#endif   // rotary_encoder_h