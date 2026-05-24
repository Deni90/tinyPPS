#ifndef rotary_encoder_h
#define rotary_encoder_h

#include <cstdint>

#include "gpio_iface.h"

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
     * Construct a rotary encoder object
     *
     * @param[in] a_gpio A pin of the rotary encoder
     * @param[in] b_gpio B pin of the rotary encoder
     * @param[in] btn_gpio Button pin of the rotary encoder
     * @param[in] clock Pointer to a variable that is used for debouncing
     */
    RotaryEncoder(IGpio* a_gpio, IGpio* b_gpio, IGpio* btn_gpio,
                  volatile uint32_t* clock);

    /**
     * @brief Initialize rotary encoder GPIO pins
     */
    auto initialize() -> void;

    /**
     * @brief Handle function for the rotary encoder.
     *
     * Call this function in a loop.
     */
    auto Handle() -> void;

    /**
     * @brief Get state of the rotary encoder
     *
     * @return state
     */
    [[nodiscard]] auto getState() const -> State { return m_state; }

    /**
     * @brief Clear state of the rotary encoder indicating that the state is
     * processed.
     */
    auto clearState() -> void { m_state = State::processed; }

  private:
    State m_state;
    volatile uint32_t* m_clock;
    IGpio* m_a_gpio;
    IGpio* m_b_gpio;
    IGpio* m_btn_gpio;
};

#endif   // rotary_encoder_h
