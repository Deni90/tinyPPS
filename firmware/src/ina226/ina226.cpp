#include "ina226.h"

static constexpr uint8_t k_configuration_cmd = 0x00;
static constexpr uint8_t k_shunt_voltage_cmd = 0x01;
static constexpr uint8_t k_bus_voltage_cmd = 0x02;
static constexpr uint8_t k_power_cmd = 0x03;
static constexpr uint8_t k_current_cmd = 0x04;
static constexpr uint8_t k_calibration_cmd = 0x05;
static constexpr uint8_t k_mask_enable_cmd = 0x06;
static constexpr uint8_t k_alert_limit_cmd = 0x07;
static constexpr uint8_t k_manufacturer_id_cmd = 0xfe;
static constexpr uint8_t k_die_id__cmd = 0xff;

Ina226::Ina226(II2c* i2c, uint8_t address, float shunt_resistor)
    : m_i2c(i2c), m_addr(address), m_shunt(shunt_resistor) {}

bool Ina226::calibrate(float max_current) {
    if (m_shunt <= 0.0f || max_current <= 0.0f) {
        return false;   // Invalid input
    }
    // m_shunt * max_current must be lower than 81.9 mV to prevent math overflow
    float shunt_voltage = max_current * m_shunt;
    if (shunt_voltage > 0.08190) {
        return false;
    }
    m_current_lsb = max_current / 32767.0f;
    uint16_t calib =
        static_cast<uint16_t>(0.00512f / (m_current_lsb * m_shunt));
    return writeRegister(k_calibration_cmd, calib);
}

int32_t Ina226::readBusVoltage() {
    uint16_t raw;
    if (!readRegister(k_bus_voltage_cmd, raw))
        return 0.0f;
    return raw * 1.25f;   // 1.25 mV per bit
}

int32_t Ina226::readCurrent() {
    uint16_t raw;
    if (!readRegister(k_current_cmd, raw))
        return 0;
    int16_t signedRaw = static_cast<int16_t>(raw);
    return static_cast<int32_t>(signedRaw * m_current_lsb * 1000.0f);   // in mA
}

uint16_t Ina226::getManufacturerID() {
    uint16_t value = 0;
    readRegister(k_manufacturer_id_cmd, value);
    return value;
}

uint16_t Ina226::getDieID() {
    uint16_t value = 0;
    readRegister(k_die_id__cmd, value);
    return value;
}

bool Ina226::readRegister(uint8_t reg, uint16_t& value) {
    if (m_i2c->writeTo(m_addr, &reg, 1) != 1)
        return false;

    uint8_t buffer[2];
    if (m_i2c->readFrom(m_addr, buffer, 2) != 2)
        return false;

    value = (buffer[0] << 8) | buffer[1];
    return true;
}

bool Ina226::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t buffer[3] = {reg, static_cast<uint8_t>(value >> 8),
                         static_cast<uint8_t>(value & 0xFF)};
    return m_i2c->writeTo(m_addr, buffer, 3) == 3;
}
