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

static constexpr uint8_t k_fault_mask = 0x70;   // OVP, OCP & OTP

static constexpr uint16_t k_current_min = 1000;             // mA
static constexpr uint16_t k_voltage_min = 3300;             // mV
static constexpr uint16_t k_srcpdo_fix_voltage_inc = 50;    // mV
static constexpr uint16_t k_srcpdo_fix_current_inc = 10;    // mA
static constexpr uint16_t k_srcpdo_pps_voltage_inc = 100;   // mV
static constexpr uint16_t k_srcpdo_pps_current_inc = 50;    // mA

static constexpr uint16_t k_rdo_fixed_current_inc = 10;   // mV
static constexpr uint16_t k_rdo_pps_voltage_inc = 20;     // mV
static constexpr uint16_t k_rdo_pps_current_inc = 50;     // mA

Ap33772::Ap33772(II2c* i2c) : m_i2c(i2c) {}

auto Ap33772::probe() -> bool {
    return writeRegister(0x00, static_cast<uint8_t>(0x00));
}

auto Ap33772::getStatus() -> IPdSink::Status {
    m_status = getStatusReg();
    IPdSink::Status status;
    if (m_status.ready) {
        status.is_ready = true;
    }
    if (m_status.newpdo) {
        status.caps_received = true;
    }
    if ((m_status.raw & k_fault_mask) != 0) {
        status.has_fault = true;
    }
    return status;
}

auto Ap33772::clearStatus() -> void {
    // Do nothing
    // AP22772s is clearing the status register after reading it.
}

auto Ap33772::getFaultDetails() -> IPdSink::Faults {
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

auto Ap33772::getTemp() -> uint8_t {
    uint8_t temp = 0;
    readRegister(k_cmd_temp, temp);
    return temp;
}

auto Ap33772::getPDSourcePowerCapabilities() -> uint8_t {
    uint8_t cnt = 0;

    if (!readRegister(k_cmd_pdonum, cnt)) {
        return 0;
    }

    m_i2c->writeTo(k_i2c_addr, &k_cmd_srcpdo, 1);
    std::array<uint8_t, k_max_pdo_entries * sizeof(SrcPdoReg)> buffer;
    m_i2c->readFrom(k_i2c_addr, buffer.data(), buffer.size());
    m_pdo_array = *reinterpret_cast<std::array<SrcPdoReg, k_max_pdo_entries>*>(
        buffer.data());

    return cnt;
}

auto Ap33772::getPdo(uint8_t index, Pdo& pdo) -> bool {
    if ((index >= k_max_pdo_entries) || (m_pdo_array[index].raw == 0)) {
        return false;
    }
    pdo.index = index;
    if (m_pdo_array[index].pps.apdo == 0x03) {
        pdo.type = PdoType::PPS;
        pdo.current_min = k_current_min;
        pdo.current_max =
            m_pdo_array[index].pps.current_max * k_srcpdo_pps_current_inc;
        pdo.current_step = k_srcpdo_pps_current_inc;
        pdo.voltage_min =
            m_pdo_array[index].pps.voltage_min * k_srcpdo_pps_voltage_inc;
        pdo.voltage_max =
            m_pdo_array[index].pps.voltage_max * k_srcpdo_pps_voltage_inc;
        pdo.voltage_step = k_rdo_pps_voltage_inc;
    } else {
        pdo.type = PdoType::FIX;
        pdo.current_min = k_current_min;
        pdo.current_max =
            m_pdo_array[index].fixed.current_max * k_srcpdo_fix_current_inc;
        pdo.current_step = k_srcpdo_fix_current_inc;
        pdo.voltage_min =
            m_pdo_array[index].fixed.voltage_max * k_srcpdo_fix_voltage_inc;
        pdo.voltage_max =
            m_pdo_array[index].fixed.voltage_max * k_srcpdo_fix_voltage_inc;
        pdo.voltage_step = 0;
    }
    return true;
}

auto Ap33772::setPdoOutput(uint8_t index, uint16_t voltage, uint16_t current)
    -> bool {
    // Handle invalid indices
    if ((index >= k_max_pdo_entries) || (voltage < k_voltage_min) ||
        (current < k_current_min)) {
        return false;
    }

    RdoReg rdo;
    rdo.raw = 0;
    uint8_t octphr_val = 0;

    if (m_pdo_array[index].pps.apdo == 0x03) {
        // if (current > (m_pdo_array[index].pps.current_max * 10)) {
        //     return false;
        // }
        rdo.pps.current_op = current / k_rdo_pps_current_inc;
        // if (voltage < (m_pdo_array[index].pps.voltage_min * 50) ||
        //     voltage > (m_pdo_array[index].pps.voltage_max * 50)) {
        //     return false;
        // }
        rdo.pps.voltage = voltage / k_rdo_pps_voltage_inc;
        rdo.pps.obj_position = index + 1;
        octphr_val = rdo.pps.current_op;
    } else {
        // if (current > (m_pdo_array[index].fixed.current_max * 10) ||
        //     voltage != (m_pdo_array[index].fixed.voltage_max) * 50) {
        //     return false;
        // }
        rdo.fixed.current_max = current / k_rdo_fixed_current_inc;
        rdo.fixed.current_op = current / k_rdo_fixed_current_inc;
        rdo.fixed.obj_position = index + 1;
        octphr_val = rdo.fixed.current_op;
    }
    writeRegister(k_cmd_rdo, rdo.raw);

    return false;
}

auto Ap33772::getStatusReg() -> Ap33772::StatusReg {
    StatusReg status;
    status.raw = 0;
    readRegister(k_cmd_status, status.raw);
    return status;
}

auto Ap33772::setMask(const MaskReg& mask) -> bool {
    return writeRegister(k_cmd_mask, mask.raw);
}

auto Ap33772::setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75,
                     uint16_t tr100) -> bool {
    return !writeRegister(k_cmd_tr25, tr25) ||
           !writeRegister(k_cmd_tr50, tr50) ||
           !writeRegister(k_cmd_tr75, tr75) ||
           !writeRegister(k_cmd_tr100, tr100);
}

auto Ap33772::readRegister(uint8_t reg, uint8_t& value) -> bool {
    if (m_i2c->writeTo(k_i2c_addr, &reg, 1) != 1) {
        return false;
    }
    if (m_i2c->readFrom(k_i2c_addr, &value, sizeof(value)) != sizeof(value)) {
        return false;
    }
    return true;
}

auto Ap33772::readRegister(uint8_t reg, uint16_t& value) -> bool {
    if (m_i2c->writeTo(k_i2c_addr, &reg, 1) != 1) {
        return false;
    }
    std::array<uint8_t, sizeof(uint16_t)> buffer;
    if (m_i2c->readFrom(k_i2c_addr, buffer.data(), buffer.size()) !=
        buffer.size()) {
        return false;
    }
    value = (buffer[0] & 0xff) | (buffer[1] << 8);
    return true;
}

auto Ap33772::writeRegister(uint8_t reg, uint8_t value) -> bool {
    std::array<uint8_t, 2> buffer = {reg, value};
    return m_i2c->writeTo(k_i2c_addr, buffer.data(), buffer.size()) ==
           buffer.size();
}

auto Ap33772::writeRegister(uint8_t reg, uint16_t value) -> bool {
    std::array<uint8_t, sizeof(uint16_t) + 1> buffer = {
        reg, static_cast<uint8_t>(value & 0xff),
        static_cast<uint8_t>(value >> 8)};
    return m_i2c->writeTo(k_i2c_addr, buffer.data(), buffer.size()) ==
           buffer.size();
}

auto Ap33772::writeRegister(uint8_t reg, uint32_t value) -> bool {
    std::array<uint8_t, sizeof(uint32_t) + 1> buffer = {
        reg, static_cast<uint8_t>(value & 0xff),
        static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value >> 16),
        static_cast<uint8_t>(value >> 24)};
    return m_i2c->writeTo(k_i2c_addr, buffer.data(), buffer.size()) ==
           buffer.size();
}
