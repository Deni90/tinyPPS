#include "rotary_encoder.h"
#include "pico/stdlib.h"

static constexpr uint32_t k_debounce_time = 50;       // ms
static constexpr uint32_t k_long_press_time = 1000;   // ms

RotaryEncoder::RotaryEncoder(IGpio* a_gpio, IGpio* b_gpio, IGpio* btn_gpio,
                             volatile uint32_t* clock)
    : m_state(RotaryEncoder::State::idle), m_a_gpio(a_gpio), m_b_gpio(b_gpio),
      m_btn_gpio(btn_gpio), m_clock(clock) {}

void RotaryEncoder::initialize() {
    m_a_gpio->pinMode(IGpio::PinMode::input);   // configure IO as input
    m_a_gpio->pullUp();                        // pull up IO

    m_b_gpio->pinMode(IGpio::PinMode::input);   // configure IO as input
    m_b_gpio->pullUp();                        // pull up IO

    m_btn_gpio->pinMode(IGpio::PinMode::input);   // configure IO as input
    m_btn_gpio->pullUp();                        // pull up IO
}

void RotaryEncoder::Handle() {
    if (!m_clock) {
        return;
    }

    static bool is_debounce_started = false;
    static bool is_button_pressed = false;
    static bool is_button_handling_started = false;

    // reset rotary_encoder state to idle if processed
    if (m_state == RotaryEncoder::State::processed) {
        // if the rotary_encoder is still pressed do nothing
        if (!m_btn_gpio->read()) {
            return;
        }
        // rotary_encoder is released (high), reset flags
        m_state = RotaryEncoder::State::idle;
        is_debounce_started = false;
        is_button_pressed = false;
        is_button_handling_started = false;
    }
    // catch the initial rotary_encoder press (low) and start debouncing
    if (!m_btn_gpio->read() && !is_debounce_started) {
        is_debounce_started = true;
        *m_clock = 0;
    }
    // handle rotary_encoder debouncing
    if (is_debounce_started) {
        // check if the rotary_encoder state already processed, if yes there is
        // nothing to do
        if (*m_clock >= k_debounce_time) {
            // after debounce time is elapsed check the rotary_encoder state
            // it is pressed if the rotary_encoder state is still low
            is_button_pressed = !m_btn_gpio->read();
        }
    }

    // Encoder routine. Updates counter if they are valid
    // and if rotated a full indent
    static uint8_t old_AB = 3;   // Lookup table index
    static int8_t encval = 0;    // Encoder value
    static const int8_t enc_states[] = {
        0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};   // Lookup table

    old_AB <<= 2;   // Remember previous state
    if (!m_a_gpio->read())
        old_AB |= 0x02;   // Add current state of pin A
    if (!m_b_gpio->read())
        old_AB |= 0x01;   // Add current state of pin B
    encval += enc_states[(old_AB & 0x0f)];
    // Update counter if encoder has rotated a full indent, that is at least 4
    // steps
    if (encval > 3) {   // Four steps forward
        encval = 0;
        if (is_button_pressed) {
            m_state = RotaryEncoder::State::rot_inc_while_btn_press;
            return;
        } else {
            m_state = RotaryEncoder::State::rot_inc;
            return;
        }
    } else if (encval < -3) {   // Four steps backwards
        encval = 0;
        if (is_button_pressed) {
            m_state = RotaryEncoder::State::rot_dec_while_btn_press;
            return;
        } else {
            m_state = RotaryEncoder::State::rot_dec;
            return;
        }
    }

    // decide betweeen short and long press
    // start timer for long press
    if (is_button_pressed && !is_button_handling_started) {
        is_button_handling_started = true;
        *m_clock = 0;
    }
    if (is_button_handling_started) {
        // if rotary_encoder is still pressed after
        if (!m_btn_gpio->read()) {
            if (*m_clock >= k_long_press_time) {
                m_state = RotaryEncoder::State::btn_long_press;
            }
        } else {
            m_state = RotaryEncoder::State::btn_short_press;
        }
    }
}