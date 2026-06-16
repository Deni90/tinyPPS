#ifndef pico_timer_hpp
#define pico_timer_hpp

#include "pico/time.h"
#include "timer.hpp"

class PicoRepeatingTimer {
  public:
    PicoRepeatingTimer() = default;

    ~PicoRepeatingTimer();

    /**
     * @brief Start a repeating timer.
     *
     * Starts the timer so that the provided callback is invoked
     * periodically with the given period.
     *
     * If the timer is already running, calling this function
     * should stop the timer and restart it with the new parameters.
     *
     * @param period_ms Timer period in milliseconds.
     * @param callback Callback function to be called on each timer expiration.
     * @param ctx User-defined context pointer passed to the callback.
     *            The pointed-to data must remain valid for the lifetime
     *            of the timer.
     *
     * @return true  Timer was successfully started.
     * @return false Failed to start the timer.
     */
    auto start(uint32_t period_ms, hal::timer::Callback callback, void* context)
        -> bool;

    /**
     * @brief Stop the repeating timer.
     *
     * Stops the timer if it is currently running.
     * After calling this function, the callback will no longer be invoked.
     */
    auto stop() -> void;

    /**
     * @brief Check whether the timer is running.
     *
     * @return true  Timer is running.
     * @return false Timer is stopped.
     */
    [[nodiscard]] auto isRunning() const -> bool;

  private:
    static auto timerThunk(repeating_timer_t* repeating_timer) -> bool;

    repeating_timer_t m_timer;
    hal::timer::Callback m_callback = nullptr;
    void* m_context = nullptr;
    bool m_running = false;
};

static_assert(hal::timer::RepeatingTimer<PicoRepeatingTimer>,
              "PicoRepeatingTimer must implement the "
              "hal::timer::RepeatingTimer concept!");

#endif   // pico_timer_hpp
