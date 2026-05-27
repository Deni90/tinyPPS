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
     */
    RotaryEncoder(IGpio& a_gpio, IGpio& b_gpio, IGpio& btn_gpio);

    /**
     * @brief Initialize rotary encoder GPIO pins
     */
    auto initialize() -> void;

    /**
     * @brief Handle function for the rotary encoder.
     *
     * Call this function in a loop.
     * @param[in] now_ms Current time in milliseconds
     */
    auto handle(uint32_t now_ms) -> void;

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
    IGpio& m_a_gpio;
    IGpio& m_b_gpio;
    IGpio& m_btn_gpio;
    uint32_t m_debounce_time_ms{0};
    uint32_t m_long_press_time_ms{0};
    bool m_is_debounce_started{false};
    bool m_is_button_pressed{false};
    bool m_is_button_handling_started{false};
    bool m_last_btn_state{true};   // make it high
};

#endif   // rotary_encoder_h
