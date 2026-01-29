#ifndef ap33772s_h
#define ap33772s_h

#include <cassert>
#include <cstdint>
#include <string>

#include "i2c_iface.h"

class Ap33772s {
  private:
    /**
     * @brief Status register definition of AP33772S
     */
    union Status {
        struct {
            uint8_t started : 1;
            uint8_t ready : 1;
            uint8_t newpdo : 1;
            uint8_t uvp : 1;
            uint8_t ovp : 1;
            uint8_t ocp : 1;
            uint8_t otp : 1;
            uint8_t : 1;
        };
        uint8_t raw;
    };

    /**
     * @brief Mask register definition of AP33772S
     *
     * The MASK register defines the enable and disable of ON and OFF for
     * various STATUS-defined events
     */
    union Mask {
        struct {
            uint8_t started_msk : 1;
            uint8_t ready_msk : 1;
            uint8_t newpdo_msk : 1;
            uint8_t uvp_msk : 1;
            uint8_t ovp_msk : 1;
            uint8_t ocp_msk : 1;
            uint8_t otp_msk : 1;
            uint8_t : 1;
        };
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
    union System {
        struct {
            uint8_t voutctl : 2;
            uint8_t : 2;
            uint8_t cmdver : 2;
            uint8_t : 2;
        };
        uint8_t raw;
    };

    /**
     * @brief SRC_SPR_PDOX register definition of AP33772S
     *
     * This union is created based on SRC_SPRandEPR_PDO_Fields struct from I˝C
     * Master Sample Code for AP33772
     */
    union SrcPdo {
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
        };
        uint16_t raw;
    };

    /**
     * @brief Definition of PD_REQMSG register
     *
     * The PD_REQMSG register is defined as the request message format to
     * initiate negotiation with the PD source.
     */
    union PdReqMsg {
        struct {
            uint16_t voltage_sel : 8;   // Bits 7:0  VOLTAGE_SEL filed
            uint16_t current_sel : 4;   // Bits 8:11 CURRENT_SEL field
            uint16_t pdo_index : 4;     // Bits 12:15 PDO_INDEX field
        };
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
    union PdMsgrlt {
        struct {
            uint8_t response : 2;
        };
        uint8_t raw;
    };

  public:
    /**
     * @brief Enumeration describing different PDO types
     */
    enum class PdoType { NONE, FIX, PPS, AVS };

    /**
     * @brief Covert PdoType enum value to string
     *
     * @param type PdoType
     */
    static constexpr const char* pdoTypeToString(PdoType type) {
        switch (type) {
        case PdoType::FIX:
            return "FIX";
        case PdoType::PPS:
            return "PPS";
        case PdoType::AVS:
            return "AVS";
        case PdoType::NONE:
        default:
            return "N/A";
        }
        return "N/A";
    }

    /**
     * @brief Definition of a PDO objet
     *
     * This struct is a more mainingful version of the SrcPdo union. It contains
     * parsed values for PdoType, min, max and step values for current and
     * voltage.
     */
    struct Pdo {
        uint8_t index;
        PdoType type;
        uint16_t voltage_min;    // mV
        uint16_t voltage_max;    // mV
        uint16_t voltage_step;   // mV
        uint16_t current_min;    // mV
        uint16_t current_max;    // mA
        uint16_t current_step;   // mA
        Pdo();
    };

    /**
     * @brief Constructor
     *
     * @param[in] i2c Pointer to i2c object
     */
    Ap33772s(II2c* i2c);

    /**
     * @brief Check if new PDO is available
     *
     * @return True if available, otherwise false
     */
    bool isNewPdoAvailable();

    /**
     * @brief Enable/Disable the output
     *
     * This function controls the NMOS switch. If enable is set to true, the
     * NMOS switch will be turned on.
     *
     * @param enable Boolean value used to enable/disable the output
     * @return True if the output is successfully enabled/disabled
     */
    bool enableOutput(bool enable);

    /**
     * @brief Get the temperature read from NTC
     *
     * @return Temperature [Celsius]
     */
    uint8_t getTemp();

    /**
     * @brief  NTC resistance values for selected temperatures
     *
     * @param tr25 The resistance value of the NTC thermistor at 25C [Ohm]
     * @param tr50 The resistance value of the NTC thermistor at 50C [Ohm]
     * @param tr75 The resistance value of the NTC thermistor at 75C [Ohm]
     * @param tr100 The resistance value of the NTC thermistor at 100C [Ohm]
     * @return True if the values are successfully set
     */
    bool setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75, uint16_t tr100);

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
    bool setVselMin(uint16_t voltage);

    /**
     * @brief Get all of the PD Source Power Capabilities
     *
     * @return The number of PD Source Power Capabilities available
     */
    uint8_t getPDSourcePowerCapabilities();

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
    bool getPdo(uint8_t index, Pdo& pdo);

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
    bool setPdoOutput(uint8_t index, uint16_t voltage, uint16_t current);

    /**
     * @brief The maximum number of PD Source Power Capabilities
     */
    static constexpr uint8_t k_max_pdo_entries = 13;

  private:
    // I2C helpers
    bool readRegister(uint8_t reg, uint8_t& value);
    bool readRegister(uint8_t reg, uint16_t& value);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool writeRegister(uint8_t reg, uint16_t value);

    Status getStatus();
    System getSystem();
    void setSystem(System s);

    II2c* m_i2c;
    SrcPdo m_pdo_array[k_max_pdo_entries];
};

#endif   // ap33772s_h