#include "pico_timer.h"

PicoRepeatingTimer::~PicoRepeatingTimer() { stop(); }

auto PicoRepeatingTimer::start(uint32_t period_ms, Callback callback,
                               void* context) -> bool {
    stop();   // ensure clean restart

    m_callback = callback;
    m_context = context;

    // Negative = periodic from now
    //
    bool is_ok =
        add_repeating_timer_ms(static_cast<int32_t>(period_ms),
                               &PicoRepeatingTimer::timerThunk, this, &m_timer);

    m_running = is_ok;
    return is_ok;
}

auto PicoRepeatingTimer::stop() -> void {
    if (m_running) {
        cancel_repeating_timer(&m_timer);
        m_running = false;
    }
}

auto PicoRepeatingTimer::isRunning() const -> bool { return m_running; }

auto PicoRepeatingTimer::timerThunk(repeating_timer_t* repeating_timer)
    -> bool {
    auto* self = static_cast<PicoRepeatingTimer*>(repeating_timer->user_data);
    if (self && self->m_callback) {
        self->m_callback(self->m_context);
    }
    return true;   // keep repeating
}
