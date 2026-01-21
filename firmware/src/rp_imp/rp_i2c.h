#ifndef rp_i2c_h
#define rp_i2c_h

#include "hardware/i2c.h"
#include "i2c_iface.h"
#include <cstdint>

class RpI2c : public II2c {
  public:
    /**
     * @brief Create RP2040 I2C object instance
     */
    RpI2c();

    /**
     * @brief Initialize the module
     *
     * @param[in] i2c I2C channel. RP2040 supports i2c0 and i2c1
     * @param[in] sda_pin GPIO pin for SDA
     * @param[in] scl_pin GPIO pin for SCL
     * @param[in] baudrate I2C baudrate in kHz
     */
    void initialize(i2c_inst_t* i2c, unsigned int sda_pin, unsigned int scl_pin,
                    unsigned int baudrate);

    /**
     * Implementation of II2c interface
     */
    int writeTo(uint8_t addr, const uint8_t* data, unsigned int len) override;
    int readFrom(uint8_t addr, uint8_t* data, unsigned int len) override;
};

#endif   // rp_i2c_h