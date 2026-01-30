#include "pico_gpio.h"

PicoGpio::PicoGpio(unsigned int io_pin) : m_pin(io_pin) { gpio_init(m_pin); }

bool PicoGpio::configure(Direction dir, Pull pull) {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_set_dir(m_pin, dir == Direction::Output);
    gpio_set_pulls(m_pin, pull == Pull::Up, pull == Pull::Down);
    return true;
}

bool PicoGpio::write(bool value) {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_put(m_pin, value);
    return true;
}

bool PicoGpio::read() {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    return gpio_get(m_pin);
}