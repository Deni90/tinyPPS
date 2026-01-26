#include "main_screen.h"

#include <iomanip>
#include <sstream>

MainScreen::MainScreen(uint16_t width, uint16_t height)
    : Screen(width, height), m_pdo_type(PdoType::NONE),
      m_supply_mode(SupplyMode::CV), m_is_output_enabled(false),
      m_temperature(0.0f), m_measured_voltage(0), m_measured_current(0),
      m_target_voltage(0), m_is_target_voltage_selected(false),
      m_target_current(0), m_is_target_current_selected(false) {}

uint8_t* MainScreen::build() {
    clear();

    // PDO type
    printString(0, 0, pdoTypeToString(m_pdo_type));

    // Temperature
    std::ostringstream temperature_stream;
    temperature_stream << std::fixed << std::setprecision(1) << m_temperature;
    printString(m_width, 0, temperature_stream.str(),
                {.align = TextAlign::right});

    // Measured voltage in V
    std::ostringstream measured_voltage_stream;
    measured_voltage_stream << std::fixed << std::setprecision(2)
                            << std::setfill('0') << std::setw(5)
                            << m_measured_voltage << "V";
    printString(m_width / 2, 0, measured_voltage_stream.str(),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target voltage in mV
    auto len = printString(31, 15, "TARGET ");
    std::ostringstream target_voltage_stream;
    target_voltage_stream << std::setfill('0') << std::setw(5)
                          << m_target_voltage;
    len += printString(31 + len, 15, target_voltage_stream.str(),
                       {.invert = m_is_target_voltage_selected});
    printString(31 + len, 15, "mV");

    // Measured current in A
    std::ostringstream measured_current_stream;
    measured_current_stream << std::fixed << std::setprecision(2)
                            << std::setfill('0') << std::setw(5)
                            << m_measured_current << "A";
    printString(m_width / 2, 25, measured_current_stream.str(),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target current in mA
    len = printString(33, 40, "TARGET ");
    std::ostringstream target_current_stream;
    target_current_stream << std::setfill('0') << std::setw(4)
                          << m_target_current;
    len += printString(33 + len, 40, target_current_stream.str(),
                       {.invert = m_is_target_current_selected});
    printString(33 + len, 40, "mA");

    // Constant voltage indicator
    drawRectangle(32, 53, 20, 11, m_supply_mode == SupplyMode::CV);
    printString(38, 54, supplyModeToString(SupplyMode::CV),
                {.invert = m_supply_mode == SupplyMode::CV});

    // Constant current indicator
    drawRectangle(53, 53, 20, 11, m_supply_mode == SupplyMode::CC);
    printString(59, 54, supplyModeToString(SupplyMode::CC),
                {.invert = m_supply_mode == SupplyMode::CC});

    // Output enable indicator
    drawRectangle(74, 53, 20, 11, m_is_output_enabled);
    printString(79, 54, "EN", {.invert = m_is_output_enabled});

    return m_frame_buffer;
}

MainScreen& MainScreen::setPdoType(PdoType type) {
    m_pdo_type = type;
    return *this;
}

MainScreen& MainScreen::setSupplyMode(SupplyMode mode) {
    m_supply_mode = mode;
    return *this;
}

MainScreen& MainScreen::setOutputEnable(bool value) {
    m_is_output_enabled = value;
    return *this;
}

MainScreen& MainScreen::setTemperature(float value) {
    m_temperature = value;
    return *this;
}

MainScreen& MainScreen::setMeasuredVoltage(float value) {
    m_measured_voltage = value;
    return *this;
}

MainScreen& MainScreen::setMeasuredCurrent(float value) {
    m_measured_current = value;
    return *this;
}

MainScreen& MainScreen::setTargetVoltage(unsigned int value) {
    m_target_voltage = value;
    return *this;
}

MainScreen& MainScreen::selectTargetVoltage(bool value) {
    m_is_target_voltage_selected = value;
    return *this;
}

MainScreen& MainScreen::setTargetCurrent(unsigned int value) {
    m_target_current = value;
    return *this;
}

MainScreen& MainScreen::selectTargetCurrent(bool value) {
    m_is_target_current_selected = value;
    return *this;
}
