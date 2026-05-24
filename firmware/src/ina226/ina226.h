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
     * @brief Averaging Mode enum
     * Averaging mode config parameter. Determines the number of samples that
     * are collected and averaged.
     */
    enum class AveragingMode {
        Samples1 = 0,
        Samples4 = 1,
        Samples16 = 2,
        Samples64 = 3,
        Samples128 = 4,
        Samples256 = 5,
        Samples512 = 6,
        Samples1024 = 7
    };

    /**
     * @brief Voltage Conversion Time enum
     * Bus Voltage Conversion Time and Shunt Voltage Conversion Time
     */
    enum class VoltageConversionTime {
        Time140_us = 0,
        Time204_us = 1,
        Time332_us = 2,
        Time588_us = 3,
        Time1100_us = 4,
        Time2100_us = 5,
        Time4200_us = 6,
        Time8300_us = 7
    };

    /**
     * @brief Operating mode settings
     * */
    enum class Mode {
        ShutDown = 0,
        ShuntTrigger = 1,
        BusTrigger = 2,
        ShuntBusTrigger = 3,
        ShuntContinuous = 5,
        BusContinuous = 6,
        ShuntBusContinuous = 7,
    };

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
     */
    Ina226(II2c* i2c, uint8_t address);

    /**
     * @brief Calibrate function
     *
     * @param[in] shunt Shunt Resistance in Ohms
     * @param[in] current_lsb_ma Current Least Significant Bit in milli Amperes.
     * Recommended values: 0.050, 0.100, 0.250, 0.500, 1, 2, 2.5
     * @param[in] current_zero_offset_ma Current Zero Offset in milli Amperes,
     * default = 0
     * @param[in] bus_v_scaling_e4 Bus Voltage Scaling Factor, default = 10000
     */
    auto calibrate(float shunt = 0.1, float current_lsb_ma = 0.1,
                   float current_zero_offset_ma = 0,
                   uint16_t bus_v_scaling_e4 = 10000) -> bool;

    /**
     * @brief Read bus voltage
     *
     * @return voltage [V]
     */
    auto getBusVoltage() -> float;

    /**
     * @brief Read shunt voltage
     *
     * @return voltage [V]
     */
    auto getShuntVoltage() -> float;

    /**
     * @brief Read current
     *
     * @return Current [A]
     */
    auto getCurrent() -> float;

    /**
     * @brief reset configuration
     */
    auto reset() -> bool;

    /**
     * @brief Get Averaging Mode
     *
     * Averaging mode determines the number of samples that are collected and
     * averaged.
     *
     * @param[out] avg Reference to averaging mode
     * @return True if the config paramer is correctly read
     */
    auto getAveragingMode(AveragingMode& avg) -> bool;

    /**
     * @brief Set Averaging Mode
     *
     * Averaging mode determines the number of samples that are collected and
     * averaged.
     *
     * @param[in] avg Averaging mode. Default is 1 sample.
     * @return True if the config paramer is correctly set
     */
    auto setAveragingMode(AveragingMode avg = AveragingMode::Samples1) -> bool;

    /**
     * @brief Get Bus Voltage Conversion Time
     *
     * Bus Voltage Conversion Time sets the conversion time for the bus voltage
     * measurement.
     *
     * @param[out] bvct Reference to Bus Voltage Conversion Time
     * @return True if the config paramer is correctly read
     */
    auto getBusVoltageConversionTime(VoltageConversionTime& bvct) -> bool;

    /**
     * @brief Set Bus Voltage Conversion Time
     *
     * Bus Voltage Conversion Time sets the conversion time for the bus voltage
     * measurement.
     *
     * @param[in] bvct  Bus Voltage Conversion Time. Default is 1.1ms
     * @return True if the config paramer is correctly read
     */
    auto setBusVoltageConversionTime(
        VoltageConversionTime bvct = VoltageConversionTime::Time1100_us)
        -> bool;

    /**
     * @brief Get Shunt Voltage Conversion Time
     *
     * Shunt Voltage Conversion Time sets the conversion time for the bus
     * voltage measurement.
     *
     * @param[out] bvct Reference to Shunt Voltage Conversion Tim
     * @return True if the config paramer is correctly read
     */
    auto getShuntVoltageConversionTime(VoltageConversionTime& svct) -> bool;

    /**
     * @brief Set Shunt Voltage Conversion Time
     *
     * Shunt Voltage Conversion Time sets the conversion time for the bus
     * voltage measurement.
     *
     * @param[in] bvct  Shunt Voltage Conversion Time. Default is 1.1ms
     * @return True if the config paramer is correctly read
     */
    auto setShuntVoltageConversionTime(
        VoltageConversionTime svct = VoltageConversionTime::Time1100_us)
        -> bool;

    /**
     * @brief Get Operating Mode
     *
     * @param[out] mode Reference to Operating Mode
     * @return True if the config paramer is correctly read
     */
    auto getMode(Mode& mode) -> bool;

    /**
     * @brief Set Operating Mode
     *
     * @param[out] mode Operating Mode
     * @return True if the config paramer is correctly read
     */
    auto setMode(Mode mode = Mode::ShuntBusContinuous) -> bool;

    /**
     * @brief Get Manufacturer ID
     *
     * @return Manufacturer ID
     */
    auto getManufacturerID() -> uint16_t;

    /**
     * @brief Get Die ID
     *
     * @return Die ID
     */
    auto getDieID() -> uint16_t;

    /**
     * Expected Manufacturer ID of INA226
     */
    static const uint16_t k_manufacturer_id = 0x5449;

    /**
     * Expected Die ID of INA226
     */
    static const uint16_t k_die_id = 0x2260;

  private:
    template <typename T> auto min(const T& left, const T& right) -> const T& {
        return (right < left) ? right : left;   // Returns the first if equal
    }
    auto readRegister(uint8_t reg, uint16_t& value) -> bool;
    auto writeRegister(uint8_t reg, uint16_t value) -> bool;

    II2c* m_i2c;
    uint8_t m_addr;
    float m_shunt;
    float m_current_lsb;
    float m_max_current;
    float m_current_zero_offset;
    uint16_t m_bus_v_scaling_e4;
};

#endif   // ina226_h
