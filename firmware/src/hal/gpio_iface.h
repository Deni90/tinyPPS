#ifndef gpio_iface_h
#define gpio_iface_h

class IGpio {
  public:
    /**
     * @brief GPIO direction.
     */
    enum class Direction { Input, Output };

    /**
     * @brief Internal pull resistor configuration.
     */
    enum class Pull { None, Up, Down };

    /**
     * @brief GPIO interrupt trigger edge.
     */
    enum class Edge { Rising, Falling, Both };

    /**
     * @brief GPIO interrupt callback type.
     *
     * @param gpio Reference to the GPIO instance that triggered the interrupt
     * @param user User-defined context pointer
     */
    using IrqCallback = void (*)(IGpio& gpio, void* user);

    /**
     * @brief Configure the GPIO pin.
     *
     * @param dir  Pin direction (input or output)
     * @param pull Internal pull resistor configuration
     *
     * @return true on success, false on invalid configuration
     */
    virtual bool configure(Direction dir, Pull pull = Pull::None) = 0;

    /**
     * @brief Write a value to a digital pin.
     *
     * @param[in] value true - high, false - low
     */
    virtual bool write(bool value) = 0;

    /**
     * @brief Reads the value from a specified digital pin.
     *
     * @return true - high, false - low
     */
    virtual bool read() = 0;

    /**
     * @brief Attach an interrupt callback to the GPIO pin.
     *
     * Configures an edge-triggered interrupt and registers a user callback.
     * The callback is executed from interrupt context.
     *
     * @warning The callback must be fast and must not block.
     *
     * @param edge Interrupt trigger edge
     * @param cb   Callback function
     * @param user Optional user-defined context pointer
     *
     * @return true on success, false if the interrupt could not be configured
     */
    virtual bool attachInterrupt(Edge edge, IrqCallback cb,
                                 void* user = nullptr) = 0;

    /**
     * @brief Enable or disable the GPIO interrupt.
     *
     * @param enable true to enable the interrupt, false to disable it
     */
    virtual void enableInterrupt(bool enable) = 0;
};

#endif   // gpio_iface_h