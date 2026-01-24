#include "ina226.h"

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

static constexpr float k_max_shunt_voltage = 81.92 / 1000;
static constexpr float k_min_shunt_ohm = 0.001f;

Ina226::Ina226(II2c* i2c, uint8_t address) : m_i2c(i2c), m_addr(address) {}

bool Ina226::calibrate(float shunt, float current_lsb_ma,
                       float current_zero_offset_mA,
                       uint16_t bus_v_scaling_e4) {
    if (shunt < k_min_shunt_ohm)
        return false;
    float max_current =
        min<float>((k_max_shunt_voltage / shunt), 32768 * current_lsb_ma * 1e-3);
    if (max_current < 0.001)
        return false;

    m_shunt = shunt;
    m_current_lsb = current_lsb_ma * 1e-3;
    m_current_zero_offset = current_zero_offset_mA * 1e-3;
    m_bus_v_scaling_e4 = bus_v_scaling_e4;
    m_max_current = max_current;

    uint32_t calib = round(0.00512 / (m_current_lsb * m_shunt));
    return writeRegister(k_cmd_calibration, calib);
}

float Ina226::getBusVoltage() {
    uint16_t val = 0;
    if (!readRegister(k_cmd_bus_voltage, val)) {
        return 0;
    }
    float voltage = val * 1.25e-3;
    if (m_bus_v_scaling_e4 != 10000) {
        voltage *= m_bus_v_scaling_e4 * 1.0e-4;
    }
    return voltage;
}

float Ina226::getShuntVoltage() {
    uint16_t val = 0;
    if (!readRegister(k_cmd_shunt_voltage, val)) {
        return 0;
    }
    return val * 2.5e-6;   //  fixed 2.50 uV
}

float Ina226::getCurrent() {
    uint16_t val = 0;
    if (!readRegister(k_cmd_current, val)) {
        return 0;
    }
    return val * m_current_lsb - m_current_zero_offset;
}

bool Ina226::reset() {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config |= k_conf_mask_reset;
    if (!writeRegister(k_cmd_configuration, config)) {
        return false;
    }
    m_current_lsb = 0;
    m_max_current = 0;
    m_shunt = 0;
    return false;
}

bool Ina226::getAveragingMode(AveragingMode& avg) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_average;
    avg = static_cast<AveragingMode>(config >> 9);
    return true;
}

bool Ina226::setAveragingMode(AveragingMode avg) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_average;
    config |= (static_cast<uint8_t>(avg) << 9);
    return writeRegister(k_cmd_configuration, config);
}

bool Ina226::getBusVoltageConversionTime(VoltageConversionTime& bvct) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_busvc;
    bvct = static_cast<VoltageConversionTime>(config >> 6);
    return true;
}

bool Ina226::setBusVoltageConversionTime(VoltageConversionTime bvct) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_busvc;
    config |= (static_cast<uint8_t>(bvct) << 6);
    return writeRegister(k_cmd_configuration, config);
}

bool Ina226::getShuntVoltageConversionTime(VoltageConversionTime& svct) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_shuntvc;
    svct = static_cast<VoltageConversionTime>(config >> 3);
    return true;
}
bool Ina226::setShuntVoltageConversionTime(VoltageConversionTime svct) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_shuntvc;
    config |= (static_cast<uint8_t>(svct) << 3);
    return writeRegister(k_cmd_configuration, config);
}

bool Ina226::getMode(Mode& mode) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= k_conf_mask_mode;
    mode = static_cast<Mode>(config);
    return true;
}

bool Ina226::setMode(Mode mode) {
    uint16_t config = 0;
    if (!readRegister(k_cmd_configuration, config)) {
        return false;
    }
    config &= ~k_conf_mask_mode;
    config |= (static_cast<uint8_t>(mode));
    return writeRegister(k_cmd_configuration, config);
}

uint16_t Ina226::getManufacturerID() {
    uint16_t value = 0;
    readRegister(k_cmd_manufacturer_id, value);
    return value;
}

uint16_t Ina226::getDieID() {
    uint16_t value = 0;
    readRegister(k_cmd_die_id, value);
    return value;
}

bool Ina226::readRegister(uint8_t reg, uint16_t& value) {
    if (m_i2c->writeTo(m_addr, &reg, 1) != 1)
        return false;
    uint8_t buffer[2];
    if (m_i2c->readFrom(m_addr, buffer, 2) != 2)
        return false;
    // Combine bytes (Big-endian)
    value = (buffer[0] << 8) | buffer[1];
    return true;
}

bool Ina226::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t buffer[3] = {reg, static_cast<uint8_t>((value >> 8) & 0xFF),
                         static_cast<uint8_t>(value & 0xFF)};
    return m_i2c->writeTo(m_addr, buffer, 3) == 3;
}