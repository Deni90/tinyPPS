#ifndef timer_hpp
#define timer_hpp

#include <concepts>
#include <cstdint>
#include <utility>

namespace hal::timer {

using Callback = void (*)(void* ctx);

/**
 * @brief Concept for a repeating timer.
 *
 * A repeating timer must provide the following methods:
 * - `bool start(uint32_t period_ms, Callback callback, void* ctx)`
 * - `void stop()`
 * - `bool is_running()`
 */
template <typename T>
concept RepeatingTimer =
    requires(T timer, uint32_t period_ms, Callback callback, void* ctx) {
        { timer.start(period_ms, callback, ctx) } -> std::same_as<bool>;
        { timer.stop() } -> std::same_as<void>;
        { std::as_const(timer).isRunning() } -> std::same_as<bool>;
    };

}   // namespace hal::timer

#endif   // timer_hpp
