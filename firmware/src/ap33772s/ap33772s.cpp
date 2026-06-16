#include "ap33772s.hpp"

#include <cstdint>
#include <cstring>
#include <utility>

/// @brief AP33772s register commands
static constexpr uint8_t k_cmd_status = 0x01;
static constexpr uint8_t k_cmd_mask = 0x02;
static constexpr uint8_t k_cmd_opmode = 0x03;
static constexpr uint8_t k_cmd_config = 0x04;
static constexpr uint8_t k_cmd_pdconfig = 0x05;
static constexpr uint8_t k_cmd_system = 0x06;
static constexpr uint8_t k_cmd_tr25 = 0x0c;
static constexpr uint8_t k_cmd_tr50 = 0x0d;
static constexpr uint8_t k_cmd_tr75 = 0x0e;
static constexpr uint8_t k_cmd_tr100 = 0x0f;
static constexpr uint8_t k_cmd_voltage = 0x11;
static constexpr uint8_t k_cmd_current = 0x12;
static constexpr uint8_t k_cmd_temp = 0x13;
static constexpr uint8_t k_cmd_vreq = 0x14;
static constexpr uint8_t k_cmd_ireq = 0x15;
static constexpr uint8_t k_cmd_vselmin = 0x16;
static constexpr uint8_t k_cmd_uvpthr = 0x17;
static constexpr uint8_t k_cmd_ovpthr = 0x18;
static constexpr uint8_t k_cmd_octphr = 0x19;
static constexpr uint8_t k_cmd_otpthr = 0x1a;
static constexpr uint8_t k_cmd_drthr = 0x1b;
static constexpr uint8_t k_cmd_srcpdo = 0x20;
static constexpr uint8_t k_cmd_src_spr_pdo1 = 0x21;
static constexpr uint8_t k_cmd_src_spr_pdo2 = 0x22;
static constexpr uint8_t k_cmd_src_spr_pdo3 = 0x23;
static constexpr uint8_t k_cmd_src_spr_pdo4 = 0x24;
static constexpr uint8_t k_cmd_src_spr_pdo5 = 0x25;
static constexpr uint8_t k_cmd_src_spr_pdo6 = 0x26;
static constexpr uint8_t k_cmd_src_spr_pdo7 = 0x27;
static constexpr uint8_t k_cmd_src_epr_pdo8 = 0x28;
static constexpr uint8_t k_cmd_src_epr_pdo9 = 0x29;
static constexpr uint8_t k_cmd_src_epr_pdo10 = 0x2a;
static constexpr uint8_t k_cmd_src_epr_pdo11 = 0x2b;
static constexpr uint8_t k_cmd_src_epr_pdo12 = 0x2c;
static constexpr uint8_t k_cmd_src_epr_pdo13 = 0x2d;
static constexpr uint8_t k_cmd_pd_reqmsg = 0x31;
static constexpr uint8_t k_cmd_pd_cmdmsg = 0x32;
static constexpr uint8_t k_cmd_pd_msgrlt = 0x33;
static constexpr uint8_t k_cmd_gpio = 0x52;

static constexpr uint8_t k_voutctl_auto = 0;   // controlled by the AP33772S
static constexpr uint8_t k_voutctl_off = 1;    // VOUT force OFF
static constexpr uint8_t k_voutctl_on = 2;     // VOUT force ON

static constexpr uint8_t k_fault_mask = 0x78;   // OVP, OCP, OTP & UVP

static constexpr uint16_t k_current_min = 1000;   // mA
static constexpr uint16_t k_voltage_min = 3300;   // mV

Ap33772s::Ap33772s(const I2c& i2c) : m_i2c(i2c) {}

auto Ap33772s::probe() -> bool {
    return writeRegister(0x00, static_cast<uint8_t>(0x00));
}

auto Ap33772s::getStatus() -> IPdSink::Status {
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

auto Ap33772s::clearStatus() -> void {
    // Do nothing
    // AP22772s is clearing the status register after reading it.
}

auto Ap33772s::getFaultDetails() -> IPdSink::Faults {
    IPdSink::Faults fault;
    if (m_status.ovp) {
        fault.over_voltage = true;
    }
    if (m_status.uvp) {
        fault.under_voltage = true;
    }
    if (m_status.ocp) {
        fault.over_current = true;
    }
    if (m_status.otp) {
        fault.over_temperature = true;
    }
    return fault;
}

auto Ap33772s::getTemp() -> uint8_t {
    uint8_t temp = 0;
    readRegister(k_cmd_temp, temp);
    return temp;
}

auto Ap33772s::getPDSourcePowerCapabilities() -> uint8_t {
    uint8_t cnt = 0;

    m_i2c.writeTo(k_i2c_addr, std::span<const uint8_t>(&k_cmd_srcpdo, 1));
    std::array<uint8_t, k_max_pdo_entries * sizeof(SrcPdoReg)> buf;
    m_i2c.readFrom(k_i2c_addr, buf);
    m_pdo_array = *reinterpret_cast<std::array<SrcPdoReg, k_max_pdo_entries>*>(
        buf.data());

    for (auto i = 0; std::cmp_less(i, k_max_pdo_entries); ++i) {
        if (m_pdo_array[i].raw != 0U) {
            ++cnt;
        }
    }
    return cnt;
}

auto Ap33772s::getPdo(uint8_t index, Pdo& pdo) -> bool {
    if ((index >= k_max_pdo_entries) || (m_pdo_array[index].raw == 0)) {
        return false;
    }
    pdo.index = index;
    pdo.current_min = k_current_min;
    pdo.current_step = 250;
    pdo.current_max =
        k_current_min +
        (m_pdo_array[index].fixed.current_max *
         250);   // TODO this is not god enough cause the
                 // m_pdo_array[index].fixed.current_max is giving ranges
    bool is_epr = (index >= 7 && index <= 12);   // 1-6 for SPR, 7-12 for EPR
    if (m_pdo_array[index].fixed.type == 0) {    // Fixed PDO
        pdo.type = PdoType::FIX;
        pdo.voltage_min =
            m_pdo_array[index].fixed.voltage_max * (is_epr ? 200 : 100);
        // for Fixed PDO set VOLTAGE_MIN = VOLTAGE_MAX
        pdo.voltage_max = pdo.voltage_min;
        pdo.voltage_step = 0;
    } else {
        pdo.type = is_epr ? PdoType::AVS : PdoType::PPS;
        pdo.voltage_step = is_epr ? 200 : 100;
        if (m_pdo_array[index].pps.voltage_min == 1) {
            pdo.voltage_min = is_epr ? 15000 : k_voltage_min;
        } else if (m_pdo_array[index].pps.voltage_min == 2) {
            // In this case:
            //               3.3V < VOLTAGE_MIN ≤ 5V for PPS
            //               15V < VOLTAGE_MIN ≤ 20V for AVS
            // To find the exact VOLTAGE_MIN it is needed to iterate through the
            // voltage range and find the first voltage that is accepted by the
            // source

            // TODO implement this
        }
        pdo.voltage_max =
            m_pdo_array[index].pps.voltage_max * (is_epr ? 200 : 100);
    }
    return true;
}

auto Ap33772s::setPdoOutput(uint8_t index, uint16_t voltage, uint16_t current)
    -> bool {
    if ((index >= k_max_pdo_entries) || (voltage < k_voltage_min) ||
        (current < k_current_min)) {
        return false;
    }
    bool is_epr = (index >= 7 && index <= 12);   // 1-6 for SPR, 7-12 for EPR
    PdReqMsgReg req;
    req.pdo_index = index + 1;
    req.voltage_sel = voltage / (is_epr ? 200 : 100);
    req.current_sel = (current / 250) - 4;

    if (!writeRegister(k_cmd_pd_reqmsg, req.raw)) {
        return false;
    }
    PdMsgrltReg res;
    if (!readRegister(k_cmd_pd_msgrlt, res.raw)) {
        return false;
    }
    if (res.response == 1) {
        return true;
    }
    return false;
}

auto Ap33772s::getStatusReg() -> Ap33772s::StatusReg {
    StatusReg status;
    status.raw = 0;
    readRegister(k_cmd_status, status.raw);
    return status;
}

auto Ap33772s::setMask(const MaskReg& mask) -> bool {
    return writeRegister(k_cmd_mask, mask.raw);
}

auto Ap33772s::setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75,
                      uint16_t tr100) -> bool {
    return !writeRegister(k_cmd_tr25, tr25) ||
           !writeRegister(k_cmd_tr50, tr50) ||
           !writeRegister(k_cmd_tr75, tr75) ||
           !writeRegister(k_cmd_tr100, tr100);
}

auto Ap33772s::setOtpThreshold(uint8_t threshold) -> bool {
    return writeRegister(k_cmd_otpthr, threshold);
}

auto Ap33772s::setVselMin(uint16_t voltage) -> bool {
    return writeRegister(k_cmd_vselmin, static_cast<uint8_t>(voltage / 200));
}

auto Ap33772s::readRegister(uint8_t reg, uint8_t& value) -> bool {
    if (m_i2c.writeTo(k_i2c_addr, std::span<const uint8_t>(&reg, 1)) != 1) {
        return false;
    }
    if (m_i2c.readFrom(k_i2c_addr, std::span<uint8_t>(&value, sizeof(value))) !=
        sizeof(value)) {
        return false;
    }
    return true;
}

auto Ap33772s::readRegister(uint8_t reg, uint16_t& value) -> bool {
    if (m_i2c.writeTo(k_i2c_addr, std::span<const uint8_t>(&reg, 1)) != 1) {
        return false;
    }
    std::array<uint8_t, 2> buf;
    if (m_i2c.readFrom(k_i2c_addr, std::span<uint8_t>(buf)) != buf.size()) {
        return false;
    }
    value = (buf[0] & 0xff) | (buf[1] << 8);
    return true;
}

auto Ap33772s::writeRegister(uint8_t reg, uint8_t value) -> bool {
    std::array<uint8_t, sizeof(uint8_t) + 1> buffer = {reg, value};
    return m_i2c.writeTo(k_i2c_addr, buffer) == buffer.size();
}

auto Ap33772s::writeRegister(uint8_t reg, uint16_t value) -> bool {
    std::array<uint8_t, sizeof(uint16_t) + 1> buffer = {
        reg, static_cast<uint8_t>(value & 0xff),
        static_cast<uint8_t>(value >> 8)};
    return m_i2c.writeTo(k_i2c_addr, buffer) == buffer.size();
}

auto Ap33772s::getSystemReg() -> Ap33772s::SystemReg {
    SystemReg system;
    system.raw = 0;
    readRegister(k_cmd_system, system.raw);
    return system;
}

auto Ap33772s::setSystemReg(SystemReg sys) -> void {
    writeRegister(k_cmd_system, sys.raw);
}
