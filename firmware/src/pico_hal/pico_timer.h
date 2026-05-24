#ifndef pico_timer_h
#define pico_timer_h

#include "pico/time.h"
#include "timer_iface.h"

class PicoRepeatingTimer final : public IRepeatingTimer {
  public:
    PicoRepeatingTimer() = default;
    ~PicoRepeatingTimer();

    /**
     * Implementation of IRepeatingTimer interface
     */
    auto start(uint32_t period_ms, Callback callback, void* context)
        -> bool override;
    auto stop() -> void override;
    [[nodiscard]] auto isRunning() const -> bool override;

  private:
    static auto timerThunk(repeating_timer_t* repeating_timer) -> bool;

    repeating_timer_t m_timer;
    Callback m_callback = nullptr;
    void* m_context = nullptr;
    bool m_running = false;
};

#endif   // pico_timer_h
