#ifndef ina226_h
#define ina226_h

#include "i2c_iface.h"

#include <cstdint>

/**
 * @brief Class representing INA226
 */
class Ina226 {
  public:
    /**
     * @brief Constructor
     * Table with Address Pins and Slave Addresses:
     *  A1  | A0  | SLAVE ADDRESS
     * ---------------------------
     *  GND | GND | 0x40
     *  GND | VS  | 0x41
     *  GND | SDA | 0x42
     *  GND | SCL | 0x43
     *  VS  | GND | 0x44
     *  VS  | VS  | 0x45
     *  VS  | SDA | 0x46
     *  VS  | SCL | 0x47
     *  SDA | GND | 0x48
     *  SDA | VS  | 0x49
     *  SDA | SDA | 0x4a
     *  SDA | SCL | 0x4b
     *  SCL | GND | 0x4c
     *  SCL | VS  | 0x4d
     *  SCL | SDA | 0x4e
     *  SCL | SCL | 0x4f
     *
     * @param[in] i2c Pointer to i2c interface implementation
     * @param[in] address I2C address of the chip
     * @param[in] shunt_resistor Shunt resistor value in Ohms. 0.1mOhm by default
     */
    Ina226(II2c* i2c, uint8_t address, float shunt_resistor = 0.1f);

    /**
     * @brief
     */
    bool calibrate(float max_current);

    /**
     * @brief Read bus voltage in millivolts
     *
     * @return voltage [mV]
     */
    int32_t readBusVoltage();

    /**
     * @brief Read current in milliampers
     *
     * @return Current [mA]
     */
    int32_t readCurrent();

    /**
     * @brief Get Manufacturer ID
     *
     * @return Manufacturer ID
     */
    uint16_t getManufacturerID();

    /**
     * @brief Get Die ID
     *
     * @return Die ID
     */
    uint16_t getDieID();

    /**
     * Expected Manufacturer ID of INA226
     */
    static const uint16_t k_manufacturer_id = 0x5449;

     /**
     * Expected Die ID of INA226
     */
    static const uint16_t k_die_id = 0x2260;

  private:
    bool readRegister(uint8_t reg, uint16_t& value);
    bool writeRegister(uint8_t reg, uint16_t value);

    II2c* m_i2c;
    uint8_t m_addr;
    float m_shunt;
    float m_current_lsb;
};

#endif   // ina226_h