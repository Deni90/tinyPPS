#include "main_screen.h"

#include <iomanip>
#include <sstream>

static const uint8_t empty_circle[] = {0x1c, 0x22, 0x41, 0x41,
                                       0x41, 0x22, 0x1c};
static const uint8_t full_circle[] = {0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c};

MainScreen::MainScreen(uint16_t width, uint16_t height)
    : Screen(width, height), m_is_led_on(false), m_mode(MainScreen::Mode::none),
      m_is_cc(false), m_is_cv(false), m_is_output_enabled(false),
      m_temperature(0.0f), m_measured_voltage(0), m_measured_current(0),
      m_target_voltage(0), m_is_target_voltage_selected(false),
      m_target_current(0), m_is_target_current_selected(false) {}

uint8_t* MainScreen::build() {
    clear();

    // LED indicator
    if (m_is_led_on) {
        draw(0, 0, full_circle, 7, 7, false);
    } else {
        draw(0, 0, empty_circle, 7, 7, false);
    }

    // Temperature
    std::ostringstream temperature_stream;
    temperature_stream << std::fixed << std::setprecision(1) << m_temperature;
    printString(m_width, 0, temperature_stream.str(),
                {.align = TextAlign::right});

    // Measured voltage. Convert mV to V.
    std::ostringstream measured_voltage_stream;
    measured_voltage_stream
        << std::fixed << std::setprecision(2) << std::setfill('0')
        << std::setw(5) << static_cast<float>(m_measured_voltage) / 1000 << "V";
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

    // Measured current. Convert mA to A.
    std::ostringstream measured_current_stream;
    measured_current_stream
        << std::fixed << std::setprecision(2) << std::setfill('0')
        << std::setw(5) << static_cast<float>(m_measured_current) / 1000 << "A";
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

    // Mode indicator
    std::string mode_str;
    switch(m_mode) {
        case MainScreen::Mode::pdo:
            mode_str = "PDO";
            break;
        case MainScreen::Mode::pps:
            mode_str = "PPS";
            break;
        case MainScreen::Mode::none:
        default:
            mode_str = "N/A";
    }
    drawRectangle(21, 53, 20, 11, true);
    printString(24, 54, mode_str, {.invert = true});

    // Constant voltage indicator
    drawRectangle(42, 53, 20, 11, m_is_cv);
    printString(48, 54, "CV", {.invert = m_is_cv});

    // Constant current indicator
    drawRectangle(63, 53, 20, 11, m_is_cc);
    printString(69, 54, "CC", {.invert = m_is_cc});

    // Output enable indicator
    drawRectangle(84, 53, 20, 11, m_is_output_enabled);
    printString(89, 54, "EN", {.invert = m_is_output_enabled});

    return m_frame_buffer;
}

MainScreen& MainScreen::setLed(bool value) {
    m_is_led_on = value;
    return *this;
}

MainScreen& MainScreen::setMode(MainScreen::Mode value) {
    m_mode = value;
    return *this;
}

MainScreen& MainScreen::setCv(bool value) {
    m_is_cv = value;
    return *this;
}

MainScreen& MainScreen::setCc(bool value) {
    m_is_cc = value;
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

MainScreen& MainScreen::setMeasuredVoltage(unsigned int value) {
    m_measured_voltage = value;
    return *this;
}

MainScreen& MainScreen::setMeasuredCurrent(unsigned int value) {
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
