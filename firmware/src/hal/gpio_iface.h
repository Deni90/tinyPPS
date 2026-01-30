#ifndef gpio_iface_h
#define gpio_iface_h

class IGpio {
  public:
    /**
     * @brief Enumeration describing pin modes
     */
    enum class PinMode { input, output };

    /**
     * @brief Set pin mode of an IO pin. It can be input or output
     *
     * @param[in] mode Flag for indicating pin mode (input or output)
     */
    virtual void pinMode(PinMode mode) = 0;

    /**
     * @brief Pull up the pin
     */
    virtual void pullUp() = 0;

    /**
     * Pull down the pin
     */
    virtual void pullDOwn() = 0;

    /**
     * @brief Write a value to a digital pin.
     *
     * @param[in] value true - high, false - low
     */
    virtual void write(bool value) = 0;

    /**
     * @brief Reads the value from a specified digital pin.
     *
     * @return true - high, false - low
     */
    virtual bool read() = 0;
};

#endif   // gpio_iface_h