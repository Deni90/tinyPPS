#include "main_screen.h"

#include "config.h"
#include "pdsink_iface.h"
#include "tiny_format.h"

auto MainScreen::build() -> FrameBuffer& {
    clear();

    // PDO type
    printString(0, 0, IPdSink::pdoTypeToString(m_pdo_type));

    // Temperature
    printString(k_width, 0, tinyFormat("%d*C", m_temperature),
                {.align = TextAlign::right});

    // Measured voltage in V
    printString(k_width / 2, 0, tinyFormat("%05.2fV", m_measured_voltage),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target/Limit voltage in mV
    std::string voltage_label =
        (m_supply_mode == SupplyMode::CV) ? "TARGET" : "LIMIT";
    auto target_voltage_pos =
        (k_width - printString(0, 0, voltage_label + " 00000mV", true)) / 2;
    auto len = printString(target_voltage_pos, 16, voltage_label + " ");
    len += printString(target_voltage_pos + len, 16,
                       tinyFormat("%05d", m_target_voltage),
                       {.invert = m_is_target_voltage_selected});
    printString(target_voltage_pos + len, 16, "mV");

    // Measured current in A
    printString(k_width / 2, 25, tinyFormat("%05.2fA", m_measured_current),
                {.align = TextAlign::center, .size = FontSize::big});

    // Target/Limit current in mA
    std::string current_label =
        (m_supply_mode == SupplyMode::CV) ? "LIMIT" : "TARGET";
    auto target_current_pos =
        (k_width - printString(0, 0, current_label + " 0000mA", true)) / 2;
    len = printString(target_current_pos, 41, current_label + " ");
    len += printString(target_current_pos + len, 41,
                       tinyFormat("%04d", m_target_current),
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

auto MainScreen::setPdoType(IPdSink::PdoType type) -> MainScreen& {
    m_pdo_type = type;
    return *this;
}

auto MainScreen::setSupplyMode(SupplyMode mode) -> MainScreen& {
    m_supply_mode = mode;
    return *this;
}

auto MainScreen::setOutputEnable(bool value) -> MainScreen& {
    m_is_output_enabled = value;
    return *this;
}

auto MainScreen::setTemperature(int value) -> MainScreen& {
    m_temperature = value;
    return *this;
}

auto MainScreen::setMeasuredVoltage(float value) -> MainScreen& {
    m_measured_voltage = value;
    return *this;
}

auto MainScreen::setMeasuredCurrent(float value) -> MainScreen& {
    m_measured_current = value;
    return *this;
}

auto MainScreen::setTargetVoltage(unsigned int value) -> MainScreen& {
    m_target_voltage = value;
    return *this;
}

auto MainScreen::selectTargetVoltage(bool value) -> MainScreen& {
    m_is_target_voltage_selected = value;
    return *this;
}

auto MainScreen::setTargetCurrent(unsigned int value) -> MainScreen& {
    m_target_current = value;
    return *this;
}

auto MainScreen::selectTargetCurrent(bool value) -> MainScreen& {
    m_is_target_current_selected = value;
    return *this;
}
