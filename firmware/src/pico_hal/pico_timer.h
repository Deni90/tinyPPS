#ifndef pico_timer_h
#define pico_timer_h

#include "pico/time.h"
#include "timer_iface.h"

class PicoRepeatingTimer final : public IRepeatingTimer {
  public:
    PicoRepeatingTimer();
    ~PicoRepeatingTimer();

/**
     * Implementation of IRepeatingTimer interface
     */
    bool start(uint32_t period_ms, Callback cb) override;
    void stop() override;
    bool isRunning() const override;

  private:
    static bool timerThunk(repeating_timer_t* rt);

    repeating_timer_t m_timer;
    Callback m_callback;
    bool m_running;
};

#endif   // pico_timer_h