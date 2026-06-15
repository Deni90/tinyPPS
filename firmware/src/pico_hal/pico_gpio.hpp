#ifndef pico_pin_hpp
#define pico_pin_hpp

#include "gpio.hpp"

class PicoGpioPin {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] io_pin RP's IO pin
     */
    constexpr PicoGpioPin(unsigned int io_pin) : m_pin(io_pin) {}

    /**
     * @brief Configure the GPIO pin.
     *
     * @param dir  Pin direction (input or output)
     * @param pull Internal pull resistor configuration
     *
     * @return true on success, false on invalid configuration
     */
    auto configure(hal::gpio::Direction dir,
                   hal::gpio::Pull pull = hal::gpio::Pull::None) const -> bool;

    /**
     * @brief Write a value to a digital pin.
     *
     * @param[in] value true - high, false - low
     */
    auto write(bool value) const -> bool;

    /**
     * @brief Reads the value from a specified digital pin.
     *
     * @return true - high, false - low
     */
    auto read() const -> bool;

    /**
     * @brief Attach an interrupt callback to the GPIO pin.
     *
     * Configures an edge-triggered interrupt and registers a user callback.
     * The callback is executed from interrupt context.
     *
     * @warning The callback must be fast and must not block.
     *
     * @param edge Interrupt trigger edge
     * @param callback   Callback function
     * @param user Optional user-defined context pointer
     *
     * @return true on success, false if the interrupt could not be
     configured
     */
    auto attachInterrupt(hal::gpio::Edge edge,
                         hal::gpio::IrqCallback<PicoGpioPin> callback,
                         void* user = nullptr) const -> bool;

    /**
     * @brief Enable or disable the GPIO interrupt.
     *
     * @param enable true to enable the interrupt, false to disable it
     */
    auto enableInterrupt(bool enable) const -> void;

  private:
    unsigned int m_pin;
};

static_assert(hal::gpio::GpioPin<PicoGpioPin>,
              "PicoGpioPin must implement hal::gpio::GpioPin concept!");

#endif   // pico_pin_hpp
