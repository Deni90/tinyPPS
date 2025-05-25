#ifndef rp_pin_h
#define rp_pin_h

#include "pico/stdlib.h"

#include "pin_iface.h"

class RpPin : public IPin {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] io_pin RP's IO pin
     */
    RpPin(unsigned int io_pin): m_io_pin(io_pin) {
        gpio_init(m_io_pin);
    }

    /**
     * Implementation of II2c interface
     */
    void pinMode(PinMode mode) override {
        if( mode == IPin::PinMode::input) {
            gpio_set_dir(m_io_pin, GPIO_IN);
        } else {
            gpio_set_dir(m_io_pin, GPIO_OUT);
        }
    }

    void pullUp() override {
        gpio_pull_up(m_io_pin);
    }

    void pullDOwn() override {
        gpio_pull_down(m_io_pin);
    }

    void write(bool value) override {
        gpio_put(m_io_pin, value);
    }

    bool read() override {
        return gpio_get(m_io_pin);
    }

  private:
    unsigned int m_io_pin;
};

#endif   // rp_pin_h