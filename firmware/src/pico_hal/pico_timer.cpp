#include "pico_timer.h"

PicoRepeatingTimer::PicoRepeatingTimer() : m_running(false) {}

PicoRepeatingTimer::~PicoRepeatingTimer() { stop(); }

bool PicoRepeatingTimer::start(uint32_t period_ms, Callback cb) {
    stop();   // ensure clean restart

    m_callback = cb;

    // Negative = periodic from now
    bool ok =
        add_repeating_timer_ms(static_cast<int32_t>(period_ms),
                               &PicoRepeatingTimer::timerThunk, this, &m_timer);

    m_running = ok;
    return ok;
}

void PicoRepeatingTimer::stop() {
    if (m_running) {
        cancel_repeating_timer(&m_timer);
        m_running = false;
    }
}

bool PicoRepeatingTimer::isRunning() const { return m_running; }

bool PicoRepeatingTimer::timerThunk(repeating_timer_t* rt) {
    auto* self = static_cast<PicoRepeatingTimer*>(rt->user_data);
    if (self && self->m_callback) {
        self->m_callback();
    }
    return true;   // keep repeating
}