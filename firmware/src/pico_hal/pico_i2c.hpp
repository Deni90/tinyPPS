#ifndef pico_i2c_hpp
#define pico_i2c_hpp

#include <cstdint>

#include "hardware/i2c.h"
#include "i2c.hpp"

class PicoI2c {
  public:
    /**
     * @brief Create RP2040 I2C object instance
     *
     * @param[in] i2c I2C channel. RP2040 supports i2c0 and i2c1
     */
    constexpr PicoI2c(i2c_inst_t* i2c) : m_i2c(i2c) {}

    /**
     * @brief Initialize the module
     *
     * @param[in] sda_pin GPIO pin for SDA
     * @param[in] scl_pin GPIO pin for SCL
     * @param[in] baudrate I2C baudrate in kHz
     */
    auto initialize(unsigned int sda_pin, unsigned int scl_pin,
                    unsigned int baudrate) const -> void;

    /**
     *  @brief Attempt to write specified number of bytes to address
     *
     * @param addr 7-bit address of device to write to
     * @param data A constant view of the data buffer to be sent
     * @return Number of bytes written, or error
     */
    auto writeTo(uint8_t addr, std::span<const uint8_t> tx_data) const -> int;

    /**
     * @brief Attempt to read specified number of bytes from address
     *
     * @param addr 7-bit address of device to read from
     * @param data A mutable view of the destination buffer where data will be
     * stored.
     * @return Number of bytes read, or error
     */
    auto readFrom(uint8_t addr, std::span<uint8_t> rx_data) const -> int;

  private:
    i2c_inst_t* m_i2c{nullptr};
};

static_assert(hal::i2c::I2c<PicoI2c>,
              "PicoI2c must implement hal::i2c::I2c concept!");

#endif   // pico_i2c_hpp
