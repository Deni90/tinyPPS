#ifndef main_screen_h
#define main_screen_h

#include "screen.h"

class MainScreen : public Screen {
  public:
    /**
     * @brief Enumeration describing different display modes
     */
    enum class Mode { none, pdo, pps };

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
     * @brief Set Mode
     *
     * @param[in] value Mode
     * @return reference to this main screen object
     */
    MainScreen& setMode(Mode value);

    /**
     * @brief Set constant voltage flag
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    MainScreen& setCv(bool value);

    /**
     * @brief Set constant current flag
     *
     * @param[in] value Flag
     * @return reference to this main screen object
     */
    MainScreen& setCc(bool value);

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
     * @param[in] value Voltage in mV
     * @return reference to this main screen object
     */
    MainScreen& setMeasuredVoltage(unsigned int value);

    /**
     * @brief Set measured current
     *
     * @param[in] value Current in mA
     * @return reference to this main screen object
     */
    MainScreen& setMeasuredCurrent(unsigned int value);

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
    Mode m_mode;
    bool m_is_cv;
    bool m_is_cc;
    bool m_is_output_enabled;
    float m_temperature;
    unsigned int m_measured_voltage;
    unsigned int m_measured_current;
    unsigned int m_target_voltage;
    bool m_is_target_voltage_selected;
    unsigned int m_target_current;
    bool m_is_target_current_selected;
};

#endif   // main_screen_h