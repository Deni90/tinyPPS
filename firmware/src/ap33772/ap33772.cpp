#include "ap33772.h"

#include <cstdint>
#include <cstring>
#include <pico/time.h>

/// @brief AP33772 register commands
static constexpr uint8_t k_cmd_srcpdo = 0x00;
static constexpr uint8_t k_cmd_pdonum = 0x1c;
static constexpr uint8_t k_cmd_status = 0x1d;
static constexpr uint8_t k_cmd_mask = 0x1e;
static constexpr uint8_t k_cmd_voltage = 0x20;
static constexpr uint8_t k_cmd_current = 0x21;
static constexpr uint8_t k_cmd_temp = 0x22;
static constexpr uint8_t k_cmd_ocpthr = 0x23;
static constexpr uint8_t k_cmd_otpthr = 0x24;
static constexpr uint8_t k_cmd_drthr = 0x25;
static constexpr uint8_t k_cmd_tr25 = 0x28;
static constexpr uint8_t k_cmd_tr50 = 0x2a;
static constexpr uint8_t k_cmd_tr75 = 0x2c;
static constexpr uint8_t k_cmd_tr100 = 0x2e;
static constexpr uint8_t k_cmd_rdo = 0x30;
static constexpr uint8_t k_cmd_vid = 0x34;
static constexpr uint8_t k_cmd_pid = 0x36;
static constexpr uint8_t k_cmd_reserved = 0x38;

Ap33772::Ap33772(II2c* i2c) : m_i2c(i2c), m_status(0) {
    memset(m_pdo_array, 0, k_max_pdo_entries * sizeof(SrcPdoReg));
}

bool Ap33772::probe() {
    return writeRegister(0x00, static_cast<uint8_t>(0x00));
}

bool Ap33772::enableOutput(bool enable) { return false; }

IPdSink::Status Ap33772::getStatus() {
    m_status = getStatusReg();
    IPdSink::Status status;
    if (m_status.ready) {
        status.is_ready = true;
    }
    if (m_status.newpdo) {
        status.caps_received = true;
    }
    if (m_status.raw & 0x70) {
        status.has_fault = true;
    }
    return status;
}

void Ap33772::clearStatus() {
    // Do nothing
    // AP22772s is clearing the status register after reading it.
}

IPdSink::Faults Ap33772::getFaultDetails() {
    IPdSink::Faults fault;
    if (m_status.ovp) {
        fault.over_voltage = true;
    }
    if (m_status.ocp) {
        fault.over_current = true;
    }
    if (m_status.otp) {
        fault.over_temperature = true;
    }
    return fault;
}

uint8_t Ap33772::getTemp() {
    uint8_t temp = 0;
    readRegister(k_cmd_temp, temp);
    return temp;
}

uint8_t Ap33772::getPDSourcePowerCapabilities() {
    uint8_t cnt = 0;

    if (!readRegister(k_cmd_pdonum, cnt)) {
        return 0;
    }

    m_i2c->writeTo(k_i2c_addr, &k_cmd_srcpdo, 1);
    uint8_t buf[k_max_pdo_entries * sizeof(SrcPdoReg)];
    m_i2c->readFrom(k_i2c_addr, buf, sizeof(buf));
    memcpy(m_pdo_array, buf, sizeof(buf));

    return cnt;
}

bool Ap33772::getPdo(uint8_t index, Pdo& pdo) {
    if ((index >= k_max_pdo_entries) || (m_pdo_array[index].raw == 0)) {
        return false;
    }
    pdo.index = index;
    if (m_pdo_array[index].pps.apdo == 0x03) {
        pdo.type = PdoType::PPS;
        pdo.current_min = 1000;
        pdo.current_max = m_pdo_array[index].pps.current_max * 50;
        pdo.current_step = 50;
        pdo.voltage_min = m_pdo_array[index].pps.voltage_min * 100;
        pdo.voltage_max = m_pdo_array[index].pps.voltage_max * 100;
        pdo.voltage_step = 20;
    } else {
        pdo.type = PdoType::FIX;
        pdo.current_min = 1000;
        pdo.current_max = m_pdo_array[index].fixed.current_max * 10;
        pdo.current_step = 10;
        pdo.voltage_min = m_pdo_array[index].fixed.voltage_max * 50;
        pdo.voltage_max = m_pdo_array[index].fixed.voltage_max * 50;
        pdo.voltage_step = 0;
    }
    return true;
}

bool Ap33772::setPdoOutput(uint8_t index, uint16_t voltage, uint16_t current) {
    // Handle invalid indices
    if ((index >= k_max_pdo_entries) || (voltage < 3300) || (current < 1000)) {
        return false;
    }

    RdoReg rdo;
    rdo.raw = 0;
    uint8_t octphr_val = 0;

    if (m_pdo_array[index].pps.apdo == 0x03) {
        // if (current > (m_pdo_array[index].pps.current_max * 10)) {
        //     return false;
        // }
        rdo.pps.current_op = current / 50;
        // if (voltage < (m_pdo_array[index].pps.voltage_min * 50) ||
        //     voltage > (m_pdo_array[index].pps.voltage_max * 50)) {
        //     return false;
        // }
        rdo.pps.voltage = voltage / 20;
        rdo.pps.obj_position = index + 1;
        octphr_val = rdo.pps.current_op;
    } else {
        if (current > (m_pdo_array[index].fixed.current_max * 10) ||
            voltage != (m_pdo_array[index].fixed.voltage_max) * 50) {
            return false;
        }
        rdo.fixed.current_max = current / 50;
        rdo.fixed.current_op = current / 50;
        rdo.fixed.obj_position = index + 1;
        octphr_val = rdo.fixed.current_op;
    }
    writeRegister(k_cmd_rdo, rdo.raw);

    return false;
}

Ap33772::StatusReg Ap33772::getStatusReg() {
    StatusReg status;
    status.raw = 0;
    readRegister(k_cmd_status, status.raw);
    return status;
}

bool Ap33772::setMask(const MaskReg& mask) {
    return writeRegister(k_cmd_mask, mask.raw);
}

bool Ap33772::setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75,
                     uint16_t tr100) {
    if (!writeRegister(k_cmd_tr25, tr25) || !writeRegister(k_cmd_tr50, tr50) ||
        !writeRegister(k_cmd_tr75, tr75) ||
        !writeRegister(k_cmd_tr100, tr100)) {
        return false;
    }
    return true;
}

bool Ap33772::readRegister(uint8_t reg, uint8_t& value) {
    if (m_i2c->writeTo(k_i2c_addr, &reg, 1) != 1)
        return false;
    if (m_i2c->readFrom(k_i2c_addr, &value, sizeof(value)) != sizeof(value))
        return false;
    return true;
}

bool Ap33772::readRegister(uint8_t reg, uint16_t& value) {
    if (m_i2c->writeTo(k_i2c_addr, &reg, 1) != 1)
        return false;
    uint8_t buf[2];
    if (m_i2c->readFrom(k_i2c_addr, buf, sizeof(buf)) != sizeof(buf))
        return false;
    value = (buf[0] & 0xff) | (buf[1] << 8);
    return true;
}

bool Ap33772::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    return m_i2c->writeTo(k_i2c_addr, buffer, sizeof(buffer)) == sizeof(buffer);
}

bool Ap33772::writeRegister(uint8_t reg, uint16_t value) {
    uint8_t buffer[3] = {reg, static_cast<uint8_t>(value & 0xff),
                         static_cast<uint8_t>(value >> 8)};
    return m_i2c->writeTo(k_i2c_addr, buffer, sizeof(buffer)) == sizeof(buffer);
}

bool Ap33772::writeRegister(uint8_t reg, uint32_t value) {
    uint8_t buffer[5] = {reg, static_cast<uint8_t>(value & 0xff),
                         static_cast<uint8_t>(value >> 8),
                         static_cast<uint8_t>(value >> 16),
                         static_cast<uint8_t>(value >> 24)};
    return m_i2c->writeTo(k_i2c_addr, buffer, sizeof(buffer)) == sizeof(buffer);
}
