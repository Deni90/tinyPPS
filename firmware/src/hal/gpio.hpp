#ifndef gpio_hpp
#define gpio_hpp

#include <concepts>

namespace hal::gpio {
/**
 * @brief Pin direction.
 */
enum class Direction { Input, Output };

/**
 * @brief Internal pull resistor configuration.
 */
enum class Pull { None, Up, Down };

/**
 * @brief Pin interrupt trigger edge.
 */
enum class Edge { Rising, Falling, Both };

/**
 * @brief Pin interrupt callback type.
 *
 * @param gpio Reference to the concrete pin instance that triggered the
 * interrupt
 * @param user User-defined context pointer
 */
template <typename T>
using IrqCallback = void (*)(const T& gpio, void* user);

/**
 * @brief Concept for a GPIO pin implementation.
 *
 * This concept defines the required interface for a GPIO pin, including
 * methods for configuration, reading, writing, and interrupt handling.
 */
template <typename T>
concept GpioPin = requires(const T pin, Direction dir, Pull pull, bool val,
                           Edge edge, IrqCallback<T> callback, void* user) {
    { pin.configure(dir, pull) } -> std::same_as<bool>;
    { pin.write(val) } -> std::same_as<bool>;
    { pin.read() } -> std::same_as<bool>;
    { pin.attachInterrupt(edge, callback, user) } -> std::same_as<bool>;
    { pin.enableInterrupt(val) } -> std::same_as<void>;
};

}   // namespace hal::gpio

#endif   // gpio_hpp
