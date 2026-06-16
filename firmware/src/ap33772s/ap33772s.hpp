#ifndef ap33772s_hpp
#define ap33772s_hpp

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#include "hardware_config.hpp"
#include "pdsink_iface.hpp"

class Ap33772s : public IPdSink {
  protected:
    /**
     * @brief Status register definition of AP33772S
     */
    union StatusReg {
        struct {
            uint8_t started : 1;
            uint8_t ready : 1;
            uint8_t newpdo : 1;
            uint8_t uvp : 1;
            uint8_t ovp : 1;
            uint8_t ocp : 1;
            uint8_t otp : 1;
            uint8_t : 1;
        } bits;
        uint8_t raw;
    };

    /**
     * @brief System register definition of AP33772S
     *
     * The SYSTEM register is defined as the system information and control
     * options that request specific functions. By default, the VOUT MOS
     * switches are controlled by the AP33772S. Writing the VOUTCTL parameter
     * can force the VOUT MOS switches to turn OFF/ON.
     */
    union SystemReg {
        struct {
            uint8_t voutctl : 2;
            uint8_t : 2;
            uint8_t cmdver : 2;
            uint8_t : 2;
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
            uint16_t voltage_max : 8;    // Bits 7:0, VOLTAGE_MAX field
            uint16_t peak_current : 2;   // Bits 9:8, PEAK_CURRENT field
            uint16_t current_max : 4;    // Bits 13:10, CURRENT_MAX field
            uint16_t type : 1;           // Bit 14, TYPE field
            uint16_t detect : 1;         // Bit 15, DETECT field
        } fixed;
        struct {
            uint16_t voltage_max : 8;   // Bits 7:0, VOLTAGE_MAX field
            uint16_t voltage_min : 2;   // Bits 9:8, VOLTAGE_MIN field
            uint16_t current_max : 4;   // Bits 13:10, CURRENT_MAX field
            uint16_t type : 1;          // Bit 14, TYPE field
            uint16_t detect : 1;        // Bit 15, DETECT field
        } pps;
        struct {
            uint16_t voltage_max : 8;   // Bits 7:0, VOLTAGE_MAX field
            uint16_t voltage_min : 2;   // Bits 9:8, VOLTAGE_MIN field
            uint16_t current_max : 4;   // Bits 13:10, CURRENT_MAX field
            uint16_t type : 1;          // Bit 14, TYPE field
            uint16_t detect : 1;        // Bit 15, DETECT field
        } avs;
        struct {
            uint8_t byte0;
            uint8_t byte1;
        } bits;
        uint16_t raw;
    };

    /**
     * @brief Definition of PD_REQMSG register
     *
     * The PD_REQMSG register is defined as the request message format to
     * initiate negotiation with the PD source.
     */
    union PdReqMsgReg {
        struct {
            uint16_t voltage_sel : 8;   // Bits 7:0  VOLTAGE_SEL filed
            uint16_t current_sel : 4;   // Bits 8:11 CURRENT_SEL field
            uint16_t pdo_index : 4;     // Bits 12:15 PDO_INDEX field
        } bits;
        uint16_t raw;
    };

    /**
     * @brief Definition of PD_MSGRLT
     *
     * The PD_MSGRLT register is defined as the message processing results of
     * PD_REQMSG and PD_CMDMSG. The RESPONSE parameter displays the message
     * response made by the AP33772S, based on the interaction result with the
     * PD source.
     */
    union PdMsgrltReg {
        struct {
            uint8_t response : 2;
        } bits;
        uint8_t raw;
    };

    /**
     * @brief Get Syetem register values
     *
     * @return System register
     */
    auto getSystemReg() -> SystemReg;

    /**
     * @brief Set Syetem register values
     *
     * @param[in] s System register
     */
    auto setSystemReg(SystemReg sys) -> void;

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
            uint8_t started_msk : 1;
            uint8_t ready_msk : 1;
            uint8_t newpdo_msk : 1;
            uint8_t uvp_msk : 1;
            uint8_t ovp_msk : 1;
            uint8_t ocp_msk : 1;
            uint8_t otp_msk : 1;
            uint8_t : 1;
        } bits;
        uint8_t raw;
    };

    /**
     * @brief Constructor
     *
     * @param[in] i2c Reference to i2c object
     */
    Ap33772s(const I2c& i2c);

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
     * @brief Sets the minimum allowed output voltage selection.
     *
     * The VSELMIN register is defined as the Minimum Selection Voltage. If the
     * VREQ voltage is more than or equal to the VSELMIN voltage, the VOUT MOS
     * switches turn ON after the system is ready (STATUS.READY = 1).
     *
     * @param voltage Minimum output voltage in millivolts (mV).
     *
     * @return true  If the minimum voltage was successfully set.
     * @return false If the voltage value is invalid or the operation failed.
     *
     * @note The voltage must be within the valid range supported by the
     *       connected USB-PD controller.
     */
    auto setVselMin(uint16_t voltage) -> bool;

    /**
     * @brief The maximum number of PD Source Power Capabilities
     */
    static constexpr uint8_t k_max_pdo_entries = 13;

    /**
     * @brief The I2C address of the device
     */
    static constexpr uint8_t k_i2c_addr = 0x52;

  private:
    // I2C helpers
    auto readRegister(uint8_t reg, uint8_t& value) -> bool;
    auto readRegister(uint8_t reg, uint16_t& value) -> bool;
    auto writeRegister(uint8_t reg, uint8_t value) -> bool;
    auto writeRegister(uint8_t reg, uint16_t value) -> bool;

    const I2c& m_i2c;
    std::array<SrcPdoReg, k_max_pdo_entries> m_pdo_array{};
    StatusReg m_status{0};
};

#endif   // ap33772s_hpp
