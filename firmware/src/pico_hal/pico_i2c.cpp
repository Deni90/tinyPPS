#include "pico_i2c.hpp"

#include "pico/stdlib.h"

static const unsigned int k_frequency_1khz = 1000;

auto PicoI2c::initialize(unsigned int sda_pin, unsigned int scl_pin,
                         unsigned int baudrate) const -> void {
    i2c_init(m_i2c, baudrate * k_frequency_1khz);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

auto PicoI2c::writeTo(uint8_t addr, std::span<const uint8_t> tx_data) const
    -> int {
    if (m_i2c == nullptr) {
        return -1;
    }
    return i2c_write_blocking(m_i2c, addr, tx_data.data(), tx_data.size(),
                              false);
}

auto PicoI2c::readFrom(uint8_t addr, std::span<uint8_t> rx_data) const -> int {
    if (m_i2c == nullptr) {
        return -1;
    }
    return i2c_read_blocking(m_i2c, addr, rx_data.data(), rx_data.size(),
                             false);
}
