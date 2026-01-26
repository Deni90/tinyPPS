#ifndef main_screen_h
#define main_screen_h

#include "ap33772s.h"
#include "config.h"
#include "screen.h"

class MainScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screeen
     */
    MainScreen(uint16_t width, uint16_t height);

    /**
     * @brief Destructor
     */
    ~MainScreen() = default;

    /**
     * @brief Build main screen based on data provided by user
     *
     * @return An array containing the main screen matching the screen
     * dimensions
     */
    virtual uint8_t* build() override;

    /**
     * @brief Set PDO type
     *
     * @param[in] type PDO type
     * @return reference to this main screen object
     */
    MainScreen& setPdoType(Ap33772s::PdoType type);

    /**
     * @brief Set Supply mode
     *
     * @param[in] mode Supply mode
     * @return reference to this main screen object
     */
    MainScreen& setSupplyMode(SupplyMode mode);

    /**
     * @brief Set output enable flag
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    MainScreen& setOutputEnable(bool value);

    /**
     * @brief Set temperature
     *
     * @param[in] value Temperature
     * @return reference to this main screen object
     */
    MainScreen& setTemperature(float value);

    /**
     * @brief Set measured voltage
     *
     * @param[in] value Voltage in V
     * @return reference to this main screen object
     */
    MainScreen& setMeasuredVoltage(float value);

    /**
     * @brief Set measured current
     *
     * @param[in] value Current in A
     * @return reference to this main screen object
     */
    MainScreen& setMeasuredCurrent(float value);

    /**
     * @brief Set target voltage
     *
     * @param[in] value Voltage in mV
     * @return reference to this main screen object
     */
    MainScreen& setTargetVoltage(unsigned int value);

    /**
     * @brief Invert target voltage to mark it selected
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    MainScreen& selectTargetVoltage(bool value);

    /**
     * @brief Set target current
     *
     * @param[in] value Current in mA
     * @return reference to this main screen object
     */
    MainScreen& setTargetCurrent(unsigned int value);

    /**
     * @brief Invert target current to mark it selected
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    MainScreen& selectTargetCurrent(bool value);

  private:
    Ap33772s::PdoType m_pdo_type;
    SupplyMode m_supply_mode;
    bool m_is_output_enabled;
    float m_temperature;
    float m_measured_voltage;
    float m_measured_current;
    unsigned int m_target_voltage;
    bool m_is_target_voltage_selected;
    unsigned int m_target_current;
    bool m_is_target_current_selected;
};

#endif   // main_screen_h