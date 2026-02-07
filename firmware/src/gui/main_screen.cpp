#include "main_screen.h"

#include <format>
#include <iomanip>

MainScreen::MainScreen(uint16_t width, uint16_t height)
    : Screen(width, height), m_pdo_type(Ap33772s::PdoType::NONE),
      m_supply_mode(SupplyMode::CV), m_is_output_enabled(false),
      m_temperature(0.0f), m_measured_voltage(0), m_measured_current(0),
      m_target_voltage(0), m_is_target_voltage_selected(false),
      m_target_current(0), m_is_target_current_selected(false) {}

uint8_t* MainScreen::build() {
    clear();

    // PDO type
    printString(0, 0, Ap33772s::pdoTypeToString(m_pdo_type));

    // Temperature
    printString(m_width, 0, std::format("{}*C", m_temperature),
                {.align = TextAlign::right});

    // Measured voltage in V
    printString(m_width / 2, 0, std::format("{:05.2f}V", m_measured_voltage),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target voltage in mV
    auto target_voltage_pos =
        (m_width - printString(0, 0, "TARGET 00000mV", true)) / 2;
    auto len = printString(target_voltage_pos, 16, "TARGET ");
    len += printString(target_voltage_pos + len, 16,
                       std::format("{:05d}", m_target_voltage),
                       {.invert = m_is_target_voltage_selected});
    printString(target_voltage_pos + len, 16, "mV");

    // Measured current in A
    printString(m_width / 2, 25, std::format("{:05.2f}A", m_measured_current),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target current in mA
    auto target_current_pos =
        (m_width - printString(0, 0, "LIMIT 0000mA", true)) / 2;
    len = printString(target_current_pos, 41, "LIMIT ");
    len += printString(target_current_pos + len, 41,
                       std::format("{:04d}", m_target_current),
                       {.invert = m_is_target_current_selected});
    printString(target_current_pos + len, 41, "mA");

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

MainScreen& MainScreen::setPdoType(Ap33772s::PdoType type) {
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

MainScreen& MainScreen::setTemperature(int value) {
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
