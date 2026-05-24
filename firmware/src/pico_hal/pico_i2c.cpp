#include "pico_i2c.h"

#include "pico/stdlib.h"

PicoI2c::PicoI2c() {}

auto PicoI2c::initialize(i2c_inst_t* i2c, unsigned int sda_pin,
                         unsigned int scl_pin, unsigned int baudrate) -> void {
    i2c_init(i2c, baudrate * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

auto PicoI2c::writeTo(uint8_t addr, const uint8_t* data, unsigned int len)
    -> int {
    return i2c_write_blocking(i2c_default, addr, data, len, false);
}

auto PicoI2c::readFrom(uint8_t addr, uint8_t* data, unsigned int len) -> int {
    return i2c_read_blocking(i2c_default, addr, data, len, false);
}
