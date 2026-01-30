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
};

#endif   // gpio_iface_h