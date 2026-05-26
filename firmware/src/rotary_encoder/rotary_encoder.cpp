#include "rotary_encoder.h"

#include <array>
#include <cstdint>

static constexpr uint32_t k_debounce_time = 50;       // ms
static constexpr uint32_t k_long_press_time = 1000;   // ms

RotaryEncoder::RotaryEncoder(IGpio* a_gpio, IGpio* b_gpio, IGpio* btn_gpio)
    : m_state(RotaryEncoder::State::idle), m_a_gpio(a_gpio), m_b_gpio(b_gpio),
      m_btn_gpio(btn_gpio) {}

auto RotaryEncoder::initialize() -> void {
    m_a_gpio->configure(IGpio::Direction::Input, IGpio::Pull::Up);
    m_b_gpio->configure(IGpio::Direction::Input, IGpio::Pull::Up);
    m_btn_gpio->configure(IGpio::Direction::Input, IGpio::Pull::Up);
}

auto RotaryEncoder::handle(uint32_t now_ms) -> void {
    // reset rotary_encoder state to idle if processed
    if (m_state == RotaryEncoder::State::processed) {
        // if the rotary_encoder is still pressed do nothing
        if (!m_btn_gpio->read()) {
            return;
        }
        // rotary_encoder is released (high), reset flags
        m_state = RotaryEncoder::State::idle;
        m_is_debounce_started = false;
        m_is_button_pressed = false;
        m_is_button_handling_started = false;
    }

    // Handle button press
    bool current_btn_state = m_btn_gpio->read();
    if (current_btn_state != m_last_btn_state) {
        m_last_btn_state = current_btn_state;
        m_debounce_time_ms = now_ms;
    } else if ((now_ms - m_debounce_time_ms) >= k_debounce_time) {
        m_is_button_pressed = !current_btn_state;
    }

    // Encoder routine. Updates counter if they are valid
    // and if rotated a full indent
    static uint8_t old_AB = 3;   // Lookup table index
    static int8_t encval = 0;    // Encoder value
    constexpr std::array<int8_t, 16> k_enc_states = {
        0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};   // Lookup table

    old_AB <<= 2;   // Remember previous state
    if (!m_a_gpio->read()) {
        old_AB |= 0x02;   // Add current state of pin A
    }
    if (!m_b_gpio->read()) {
        old_AB |= 0x01;   // Add current state of pin B
    }
    encval += k_enc_states[(old_AB & 0x0f)];
    // Update counter if encoder has rotated a full indent, that is at least 4
    // steps
    if (encval > 3) {   // Four steps forward
        encval = 0;
        if (m_is_button_pressed) {
            m_state = RotaryEncoder::State::rot_inc_while_btn_press;
            return;
        }
        m_state = RotaryEncoder::State::rot_inc;
        return;
    }
    if (encval < -3) {   // Four steps backwards
        encval = 0;
        if (m_is_button_pressed) {
            m_state = RotaryEncoder::State::rot_dec_while_btn_press;
            return;
        }
        m_state = RotaryEncoder::State::rot_dec;
        return;
    }

    // decide betweeen short and long press
    // start timer for long press
    if (m_is_button_pressed && !m_is_button_handling_started) {
        m_is_button_handling_started = true;
        m_long_press_time_ms = now_ms;
    }
    if (m_is_button_handling_started) {
        // if rotary_encoder is still pressed after
        if (!m_btn_gpio->read()) {
            if ((now_ms - m_long_press_time_ms) >= k_long_press_time) {
                m_state = RotaryEncoder::State::btn_long_press;
            }
        } else {
            m_state = RotaryEncoder::State::btn_short_press;
        }
    }
}
