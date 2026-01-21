#include "rp_i2c.h"

#include "pico/stdlib.h"

RpI2c::RpI2c() {}

void RpI2c::initialize(i2c_inst_t* i2c, unsigned int sda_pin,
                       unsigned int scl_pin, unsigned int baudrate) {
    i2c_init(i2c, baudrate * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

int RpI2c::writeTo(uint8_t addr, const uint8_t* data, unsigned int len) {
    return i2c_write_blocking(i2c_default, addr, data, len, false);
}

int RpI2c::readFrom(uint8_t addr, uint8_t* data, unsigned int len) {
    return i2c_read_blocking(i2c_default, addr, data, len, false);
}