#ifndef pico_i2c_h
#define pico_i2c_h

#include "hardware/i2c.h"
#include "i2c_iface.h"
#include <cstdint>

class PicoI2c : public II2c {
  public:
    /**
     * @brief Create RP2040 I2C object instance
     */
    PicoI2c();

    ~PicoI2c() final = default;

    /**
     * @brief Initialize the module
     *
     * @param[in] i2c I2C channel. RP2040 supports i2c0 and i2c1
     * @param[in] sda_pin GPIO pin for SDA
     * @param[in] scl_pin GPIO pin for SCL
     * @param[in] baudrate I2C baudrate in kHz
     */
    auto initialize(i2c_inst_t* i2c, unsigned int sda_pin, unsigned int scl_pin,
                    unsigned int baudrate) -> void;

    /**
     * Implementation of II2c interface
     */
    auto writeTo(uint8_t addr, const uint8_t* data, unsigned int len)
        -> int override;
    auto readFrom(uint8_t addr, uint8_t* data, unsigned int len)
        -> int override;
};

#endif   // pico_i2c_h
