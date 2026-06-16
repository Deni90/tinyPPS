#ifndef ap33772_hpp
#define ap33772_hpp

#include <array>
#include <cstdint>

#include "hardware_config.hpp"
#include "pdsink_iface.hpp"

class Ap33772 : public IPdSink {
  protected:
    /**
     * @brief Status register definition of AP33772S
     */
    union StatusReg {
        struct {
            uint8_t ready : 1;
            uint8_t success : 1;
            uint8_t newpdo : 1;
            uint8_t : 1;
            uint8_t ovp : 1;
            uint8_t ocp : 1;
            uint8_t otp : 1;
            uint8_t dr : 1;
        } bits;
        uint8_t raw;
    };

    /**
     * @brief SRC_SPR_PDOX register definition of AP33772S
     *
     * This union is created based on SRC_SPRandEPR_PDO_Fields struct from I˝C
     * Master Sample Code for AP33772
     */
    union SrcPdoReg {
        struct {
            uint32_t current_max
                : 10;   // Bits 9:0, Maximum Current in 10mA units
            uint32_t voltage_max : 10;   // Bits 19:10, Voltage in 50mV units
            uint32_t : 10;               // Bits 29:20 Reaserved
            uint32_t type : 2;           // Bits 31:30 00b - Fixed supply
        } fixed;
        struct {
            uint32_t current_max : 7;   // Bits 7:0, Maximum Current in 50mA
                                        // increments
            uint32_t : 1;               // Bit 7 Reserved
            uint32_t voltage_min : 8;   // Bits 15:8 Minimum Voltage in 100mV
                                        // increments
            uint32_t : 1;               // Bit 16 Reserved
            uint32_t voltage_max : 8;   // Bits 24:17  Maximum Voltage in 100mV
                                        // increments
            uint32_t : 3;               // Bits 27:25 Reserved
            uint32_t type : 2;   // Bits 29:28 00b – Programmable Power Supply,
                                 // 01b…11b - Reserved
            uint32_t apdo : 2;   // Bits 31-30 11b – Augmented Power Data Object
                                 // (APDO)
        } pps;
        uint32_t raw;
    };

    /**
     * @brief Request Data Object register definition
     */
    union RdoReg {
        struct {
            uint32_t current_max : 10;   // Bits 9:0 Maximum Operating Current
                                         // 10mA units
            uint32_t current_op
                : 10;       // Bits 19:10 Operating Current in 10mA units
            uint32_t : 8;   // Bits 27:20 Reserved - Shall be set to zero
            uint32_t obj_position : 3;   // Bits 30:28 Object position (000b is
                                         // Reserved and Shall Not be used)
            uint32_t : 1;   // Bit 31 Object position (000b is Reserved and
                            // Shall Not be used)
        } fixed;
        struct {
            uint32_t current_op : 7;   // Bits 6:0 Operating Current 50mA units
            uint32_t : 2;            // Bits 8:7 Reserved - Shall be set to zero
            uint32_t voltage : 11;   // Bits 19:9 Output Voltage in 20mV units
            uint32_t : 8;   // Bits 27:20 Reserved - Shall be set to zero
            uint32_t obj_position : 3;   // Bits 30:28 Object position (000b is
                                         // Reserved and Shall Not be used)
            uint32_t : 1;   // Bits 31 Reserved – Shall be set to zero
        } pps;
        uint32_t raw;
    };

    /**
     * @brief Get Status register values
     *
     * The STATUS register will be reset to 0 after a read operation.
     *
     * @return Status register
     */
    auto getStatusReg() -> StatusReg;

  public:
    /**
     * @brief Mask register definition of AP33772S
     *
     * The MASK register defines the enable and disable of ON and OFF for
     * various STATUS-defined events
     */
    union MaskReg {
        struct {
            uint8_t ready_en : 1;
            uint8_t success_en : 1;
            uint8_t newpdo_en : 1;
            uint8_t : 1;
            uint8_t ovp_en : 1;
            uint8_t ocp_en : 1;
            uint8_t otp_en : 1;
            uint8_t dr_en : 1;
        } bits;
        uint8_t raw;
    };

    /**
     * @brief Constructor
     *
     * @param[in] i2c Reference to i2c object
     */
    Ap33772(const I2c& i2c);

    /**
     * @brief Checks the I2C bus to see if the specific chip is present.
     *
     * @return true if the expected hardware is detected, false otherwise.
     */
    auto probe() -> bool override;

    /**
     * @brief Get status of the PD sink
     *
     * @return A Status structure
     */
    auto getStatus() -> IPdSink::Status override;

    /**
     * @brief Resets the current fault flags and clears the status of the PD
     * Sink.
     *
     * This should be called after a fault has been handled to resume normal
     * operation.
     */
    auto clearStatus() -> void override;

    /**
     * @brief Retrieves detailed hardware fault flags from the PD Sink.
     *
     * @return Faults structure containing the current state of all monitored
     * fault conditions.
     */
    auto getFaultDetails() -> Faults override;

    /**
     * @brief Get the temperature read from NTC
     *
     * @return Temperature [Celsius]
     */
    auto getTemp() -> uint8_t override;

    /**
     * @brief Get all of the PD Source Power Capabilities
     *
     * @return The number of PD Source Power Capabilities available
     */
    auto getPDSourcePowerCapabilities() -> uint8_t override;

    /**
     * @brief Retrieves the Power Data Object (PDO) at the specified index.
     *
     * This function populates the provided `Pdo` structure with voltage,
     * current, and other relevant data.
     *
     * @param index The PDO index to retrieve (starting from 0).
     * @param[out] pdo Reference to a `Pdo` structure that will be filled with
     * the retrieved data.
     *
     * @return true  If the PDO was successfully populated.
     * @return false If the PDO index is invalid
     */
    auto getPdo(uint8_t index, Pdo& pdo) -> bool override;

    /**
     * @brief Selects a Power Data Object (PDO) and sets the output voltage and
     * current.
     *
     * This function configures the power supply to use the specified PDO index
     * and applies the requested voltage and current.
     *
     * @param index   The PDO index to select (starting from 0).
     * @param voltage Desired output voltage in millivolts (mV).
     * @param current Desired output current in milliamps (mA).
     *
     * @return true  If the PDO was successfully applied.
     * @return false If the PDO selection or voltage/current setting failed.
     *
     * @note Ensure that the requested voltage and current are within the limits
     *       of the selected PDO to prevent unexpected behavior.
     */
    auto setPdoOutput(uint8_t index, uint16_t voltage, uint16_t current)
        -> bool override;

    /**
     * @brief Set Mask, used for setting up interrupt events
     *
     * @return True if successfully set
     */
    auto setMask(const MaskReg& mask) -> bool;

    /**
     * @brief  NTC resistance values for selected temperatures
     *
     * @param tr25 The resistance value of the NTC thermistor at 25C [Ohm]
     * @param tr50 The resistance value of the NTC thermistor at 50C [Ohm]
     * @param tr75 The resistance value of the NTC thermistor at 75C [Ohm]
     * @param tr100 The resistance value of the NTC thermistor at 100C [Ohm]
     * @return True if the values are successfully set
     */
    auto setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75, uint16_t tr100)
        -> bool;

    /**
     * @brief Set the OTP threshold value
     *
     * @param threshold The OTP threshold value in degrees Celsius to set
     * @return True if the value is successfully set
     */
    auto setOtpThreshold(uint8_t threshold) -> bool;

    /**
     * @brief The maximum number of PD Source Power Capabilities
     */
    static constexpr uint8_t k_max_pdo_entries = 7;

    /**
     * @brief The I2C address of the device
     */
    static constexpr uint8_t k_i2c_addr = 0x51;

  private:
    // I2C helpers
    auto readRegister(uint8_t reg, uint8_t& value) -> bool;
    auto readRegister(uint8_t reg, uint16_t& value) -> bool;
    auto writeRegister(uint8_t reg, uint8_t value) -> bool;
    auto writeRegister(uint8_t reg, uint16_t value) -> bool;
    auto writeRegister(uint8_t reg, uint32_t value) -> bool;

    const I2c& m_i2c;
    std::array<SrcPdoReg, k_max_pdo_entries> m_pdo_array;
    StatusReg m_status{0};
};

#endif   // ap33772_hpp
