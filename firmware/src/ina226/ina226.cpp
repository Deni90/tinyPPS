#include "ina226.hpp"

#include <array>
#include <cmath>

static constexpr uint8_t k_cmd_configuration = 0x00;
static constexpr uint8_t k_cmd_shunt_voltage = 0x01;
static constexpr uint8_t k_cmd_bus_voltage = 0x02;
static constexpr uint8_t k_cmd_power = 0x03;
static constexpr uint8_t k_cmd_current = 0x04;
static constexpr uint8_t k_cmd_calibration = 0x05;
static constexpr uint8_t k_cmd_mask_enable = 0x06;
static constexpr uint8_t k_cmd_alert_limit = 0x07;
static constexpr uint8_t k_cmd_manufacturer_id = 0xfe;
static constexpr uint8_t k_cmd_die_id = 0xff;

static constexpr uint16_t k_conf_mask_reset = 0x8000;
static constexpr uint16_t k_conf_mask_average = 0x0E00;
static constexpr uint16_t k_conf_mask_busvc = 0x01C0;
static constexpr uint16_t k_conf_mask_shuntvc = 0x0038;
static constexpr uint16_t k_conf_mask_mode = 0x0007;

static constexpr float k_max_shunt_voltage = 81.92e-3F;
static constexpr float k_adc_resolution = 32768.0F;
static constexpr float k_internal_calibration_multiplier = 5.12e-3F;
static constexpr float k_bus_voltage_lsb = 1.25e-3F;
static constexpr float k_shunt_voltage_lsb = 2.5e-6F;

Ina226::Ina226(const I2c& i2c, uint8_t address) : m_i2c(i2c), m_addr(address) {}

auto Ina226::calibrate(float max_current, float shunt) -> bool {
    if (max_current <= 0.0F || shunt <= 0.0F) {
        return false;
    }
    // Prevent overflow by checking the hardware shunt voltage limit (81.92 mV)
    if (max_current * shunt > k_max_shunt_voltage) {
        return false;
    }
    // INA226 Data Sheet - 6.5 Programming
    // (2) Calculate Current LSB (Maximum current divided over 2^15)
    m_current_lsb = max_current / k_adc_resolution;
    // (1) Compute Calibration Register value
    float calib_raw =
        k_internal_calibration_multiplier / (m_current_lsb * shunt);
    auto calib_val = static_cast<uint16_t>(std::round(calib_raw));

    return writeRegister(k_cmd_calibration, calib_val);
}

auto Ina226::getBusVoltage() -> float {
    uint16_t val = 0;
    if (!readRegister(k_cmd_bus_voltage, val)) {
        return 0;
    }
    // INA226 Data Sheet - 6.3.1 Basic ADC Functions
    return val * k_bus_voltage_lsb;
}

auto Ina226::getShuntVoltage() -> float {
    uint16_t val = 0;
    if (!readRegister(k_cmd_shunt_voltage, val)) {
        return 0;
    }
    // INA226 Data Sheet - 7.1.2 Shunt Voltage Register (01h) (Read-Only)
    auto signed_val = static_cast<int16_t>(val);
    return signed_val * k_shunt_voltage_lsb;
}

auto Ina226::getCurrent() -> float {
    uint16_t val = 0;
    if (!readRegister(k_cmd_current, val)) {
        return 0.0F;
    }
    // Cast the raw bits to a signed 16-bit integer to preserve the negative
    // sign
    auto signed_val = static_cast<int16_t>(val);
    return signed_val * m_current_lsb;
}

auto Ina226::reset() -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config |= k_conf_mask_reset;
    if (!writeRegister(k_cmd_configuration, config)) {
        return false;
    }
    m_current_lsb = 0.0F;
    return false;
}

auto Ina226::getAveragingMode(AveragingMode& avg) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_average;
    avg = static_cast<AveragingMode>(config >> 9);
    return true;
}

auto Ina226::setAveragingMode(AveragingMode avg) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_average;
    config |= (static_cast<uint8_t>(avg) << 9);
    return writeRegister(k_cmd_configuration, config);
}

auto Ina226::getBusVoltageConversionTime(VoltageConversionTime& bvct) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_busvc;
    bvct = static_cast<VoltageConversionTime>(config >> 6);
    return true;
}

auto Ina226::setBusVoltageConversionTime(VoltageConversionTime bvct) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_busvc;
    config |= (static_cast<uint8_t>(bvct) << 6);
    return writeRegister(k_cmd_configuration, config);
}

auto Ina226::getShuntVoltageConversionTime(VoltageConversionTime& svct)
    -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_shuntvc;
    svct = static_cast<VoltageConversionTime>(config >> 3);
    return true;
}

auto Ina226::setShuntVoltageConversionTime(VoltageConversionTime svct) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_shuntvc;
    config |= (static_cast<uint8_t>(svct) << 3);
    return writeRegister(k_cmd_configuration, config);
}

auto Ina226::getMode(Mode& mode) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_mode;
    mode = static_cast<Mode>(config);
    return true;
}

auto Ina226::setMode(Mode mode) -> bool {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_mode;
    config |= (static_cast<uint8_t>(mode));
    return writeRegister(k_cmd_configuration, config);
}

auto Ina226::getManufacturerID() -> uint16_t {
    uint16_t value = 0;
    readRegister(k_cmd_manufacturer_id, value);
    return value;
}

auto Ina226::getDieID() -> uint16_t {
    uint16_t value = 0;
    readRegister(k_cmd_die_id, value);
    return value;
}

auto Ina226::readRegister(uint8_t reg, uint16_t& value) -> bool {
    if (m_i2c.writeTo(m_addr, std::span<const uint8_t>(&reg, 1)) != 1) {
        return false;
    }
    std::array<uint8_t, 2> buffer;
    auto bytes_read = m_i2c.readFrom(m_addr, buffer);
    if (bytes_read < 0 ||
        static_cast<std::size_t>(bytes_read) != buffer.size()) {
        return false;
    }
    // Combine bytes (Big-endian)
    value = (buffer[0] << 8) | buffer[1];
    return true;
}

auto Ina226::writeRegister(uint8_t reg, uint16_t value) -> bool {
    std::array<uint8_t, 3> buffer = {reg,
                                     static_cast<uint8_t>((value >> 8) & 0xFF),
                                     static_cast<uint8_t>(value & 0xFF)};
    auto bytes_written = m_i2c.writeTo(m_addr, buffer);
    if (bytes_written < 0 ||
        static_cast<std::size_t>(bytes_written) != buffer.size()) {
        return false;
    }
    return true;
}
