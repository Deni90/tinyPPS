#ifndef main_screen_h
#define main_screen_h

#include "config.h"
#include "pdsink_iface.h"
#include "screen.h"

class MainScreen : public Screen {
  public:
    /**
     * @brief Constructor
     */
    MainScreen() = default;

    /**
     * @brief Destructor
     */
    ~MainScreen() override = default;

    /**
     * @brief Build main screen based on data provided by user
     *
     * @return A reference to shared FrameBuffer matching the display
     * dimensions.
     */
    auto build() -> FrameBuffer& override;

    /**
     * @brief Set PDO type
     *
     * @param[in] type PDO type
     * @return reference to this main screen object
     */
    auto setPdoType(IPdSink::PdoType type) -> MainScreen&;

    /**
     * @brief Set Supply mode
     *
     * @param[in] mode Supply mode
     * @return reference to this main screen object
     */
    auto setSupplyMode(SupplyMode mode) -> MainScreen&;

    /**
     * @brief Set output enable flag
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    auto setOutputEnable(bool value) -> MainScreen&;

    /**
     * @brief Set temperature
     *
     * @param[in] value Temperature
     * @return reference to this main screen object
     */
    auto setTemperature(int value) -> MainScreen&;

    /**
     * @brief Set measured voltage
     *
     * @param[in] value Voltage in V
     * @return reference to this main screen object
     */
    auto setMeasuredVoltage(float value) -> MainScreen&;

    /**
     * @brief Set measured current
     *
     * @param[in] value Current in A
     * @return reference to this main screen object
     */
    auto setMeasuredCurrent(float value) -> MainScreen&;

    /**
     * @brief Set target voltage
     *
     * @param[in] value Voltage in mV
     * @return reference to this main screen object
     */
    auto setTargetVoltage(unsigned int value) -> MainScreen&;

    /**
     * @brief Invert target voltage to mark it selected
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    auto selectTargetVoltage(bool value) -> MainScreen&;

    /**
     * @brief Set target current
     *
     * @param[in] value Current in mA
     * @return reference to this main screen object
     */
    auto setTargetCurrent(unsigned int value) -> MainScreen&;

    /**
     * @brief Invert target current to mark it selected
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    auto selectTargetCurrent(bool value) -> MainScreen&;

  private:
    IPdSink::PdoType m_pdo_type{IPdSink::PdoType::NONE};
    SupplyMode m_supply_mode{SupplyMode::CV};
    bool m_is_output_enabled{false};
    int m_temperature{0};
    float m_measured_voltage{0.0F};
    float m_measured_current{0.0F};
    unsigned int m_target_voltage{0};
    bool m_is_target_voltage_selected{false};
    unsigned int m_target_current{0};
    bool m_is_target_current_selected{false};
};

#endif   // main_screen_h
