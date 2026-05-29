#include "pico_i2c.h"

#include "pico/stdlib.h"

static const unsigned int k_frequency_1khz = 1000;

auto PicoI2c::initialize(i2c_inst_t* i2c, unsigned int sda_pin,
                         unsigned int scl_pin, unsigned int baudrate) -> void {
    m_i2c = i2c;
    i2c_init(i2c, baudrate * k_frequency_1khz);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

auto PicoI2c::writeTo(uint8_t addr, const uint8_t* data, unsigned int len)
    -> int {
    if (m_i2c == nullptr) {
        return -1;
    }
    return i2c_write_blocking(m_i2c, addr, data, len, false);
}

auto PicoI2c::readFrom(uint8_t addr, uint8_t* data, unsigned int len) -> int {
    if (m_i2c == nullptr) {
        return -1;
    }
    return i2c_read_blocking(m_i2c, addr, data, len, false);
}
