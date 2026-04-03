#ifndef pdsink_iface_h
#define pdsink_iface_h

#include <cstdint>

/**
 * @brief Interface for a USB Power Delivery (PD) Sink.
 *
 * This API defines the mandatory hardware control and monitoring functions
 * for a device acting as a power consumer (Sink) in a USB PD contract.
 */
class IPdSink {
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
     * @brief Current operational status of the PD Sink.
     */
    struct Status {
        bool is_ready = false;   // PD negotiation successful / Power stable
        bool caps_received = false;   // Source PDOs are now available
        bool has_fault = false;       // Any protection (OVP/OCP/OTP) tripped
    };

    /**
     * @brief Represents specific hardware fault conditions for a PD Sink.
     */
    struct Faults {
        bool over_voltage = false;
        bool under_voltage = false;
        bool over_current = false;
        bool over_temperature = false;
    };

    /**
     * @brief Destructor for the interface.
     */
    virtual ~IPdSink() = default;

    /**
     * @brief Checks the I2C bus to see if the specific chip is present.
     *
     * @return true if the expected hardware is detected, false otherwise.
     */
    virtual bool probe() = 0;

    /**
     * @brief Enable/Disable the output
     *
     * @param enable Boolean value used to enable/disable the output
     * @return True if the output is successfully enabled/disabled
     */
    virtual bool enableOutput(bool enable) = 0;

    /**
     * @brief Get status of the PD sink
     *
     * @return A Status structure
     */
    virtual Status getStatus() = 0;

    /**
     * @brief Resets the current fault flags and clears the status of the PD
     * Sink.
     *
     * This should be called after a fault has been handled to resume normal
     * operation.
     */
    virtual void clearStatus() = 0;

    /**
     * @brief Retrieves detailed hardware fault flags from the PD Sink.
     *
     * @return Faults structure containing the current state of all monitored
     * fault conditions.
     */
    virtual Faults getFaultDetails() = 0;

    /**
     * @brief Get the temperature read from NTC
     *
     * @return Temperature [Celsius]
     */
    virtual uint8_t getTemp() = 0;

    /**
     * @brief Get all of the PD Source Power Capabilities
     *
     * @return The number of PD Source Power Capabilities available
     */
    virtual uint8_t getPDSourcePowerCapabilities() = 0;

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
    virtual bool getPdo(uint8_t index, Pdo& pdo) = 0;

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
    virtual bool setPdoOutput(uint8_t index, uint16_t voltage,
                              uint16_t current) = 0;
};

#endif   // pdsink_iface_h
