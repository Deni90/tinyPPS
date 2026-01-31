#ifndef timer_iface_h
#define timer_iface_h

#include <cstdint>

class IRepeatingTimer {
  public:
    using Callback = void (*)(void* ctx);

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
     * @param cb Callback function to be called on each timer expiration.
     * @param ctx User-defined context pointer passed to the callback.
     *            The pointed-to data must remain valid for the lifetime
     *            of the timer.
     *
     * @return true  Timer was successfully started.
     * @return false Failed to start the timer.
     */
    virtual bool start(uint32_t period_ms, Callback cb, void* ctx) = 0;

    /**
     * @brief Stop the repeating timer.
     *
     * Stops the timer if it is currently running.
     * After calling this function, the callback will no longer be invoked.
     */
    virtual void stop() = 0;

    /**
     * @brief Check whether the timer is running.
     *
     * @return true  Timer is running.
     * @return false Timer is stopped.
     */
    virtual bool isRunning() const = 0;
};

#endif   // timer_iface_h
