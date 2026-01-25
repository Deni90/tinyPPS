#include "ap33772s.h"

#include <cstring>

static constexpr uint8_t k_i2c_addr = 0x52;

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

Ap33772s::Ap33772s(II2c* i2c) : m_i2c(i2c) {
    memset(m_pdo_array, 0, k_max_pdo_entries * sizeof(SrcPdo));
}

bool Ap33772s::isNewPdoAvailable() { return getStatus().newpdo; }

bool Ap33772s::enableOutput(bool enable) {
    Ap33772s::System system;
    // When enable is true, set VOUTCTL to Auto VOUT Control to have OVP, UVP,
    // ... protections active. With VOUT force ON they won't work.
    system.voutctl = enable ? k_voutctl_auto : k_voutctl_off;
    setSystem(system);
    return true;
}

bool Ap33772s::setNtc(uint16_t tr25, uint16_t tr50, uint16_t tr75,
                      uint16_t tr100) {
    constexpr const uint8_t k_command_length = 3;
    constexpr const uint8_t k_buffer_length = 4 * k_command_length;
    uint8_t buffer[] = {k_cmd_tr25,
                        static_cast<uint8_t>(tr25 & 0xFF),
                        static_cast<uint8_t>(tr25 >> 8),
                        k_cmd_tr50,
                        static_cast<uint8_t>(tr50 & 0xFF),
                        static_cast<uint8_t>(tr50 >> 8),
                        k_cmd_tr75,
                        static_cast<uint8_t>(tr75 & 0xFF),
                        static_cast<uint8_t>(tr75 >> 8),
                        k_cmd_tr100,
                        static_cast<uint8_t>(tr100 & 0xFF),
                        static_cast<uint8_t>(tr100 >> 8)};
    for (int i = 0; i < k_buffer_length; i += k_command_length) {
        if (!m_i2c->writeTo(k_i2c_addr, buffer + i, k_command_length)) {
            return false;
        }
    }
    return true;
}

uint8_t Ap33772s::getTemp() {
    uint8_t temp = 0;
    m_i2c->writeTo(k_i2c_addr, &k_cmd_temp, 1);
    m_i2c->readFrom(k_i2c_addr, &temp, 1);
    return temp;
}

uint8_t Ap33772s::getPDSourcePowerCapabilities() {
    uint8_t cnt = 0;

    m_i2c->writeTo(k_i2c_addr, &k_cmd_srcpdo, 1);
    uint8_t buf[k_max_pdo_entries * 2];
    m_i2c->readFrom(k_i2c_addr, buf, sizeof(buf));
    memcpy(m_pdo_array, buf, sizeof(buf));

    for (int i = 0; i < k_max_pdo_entries; ++i) {
        if (m_pdo_array[i].raw) {
            ++cnt;
        }
    }
    return cnt;
}

Ap33772s::Status Ap33772s::getStatus() {
    Status status;
    status.raw = 0;
    m_i2c->writeTo(k_i2c_addr, &k_cmd_status, 1);
    m_i2c->readFrom(k_i2c_addr, &status.raw, 1);
    return status;
}

Ap33772s::System Ap33772s::getSystem() {
    Ap33772s::System system;
    system.raw = 0;
    m_i2c->writeTo(k_i2c_addr, &k_cmd_system, 1);
    m_i2c->readFrom(k_i2c_addr, &system.raw, 1);
    return system;
}

void Ap33772s::setSystem(System s) {
    uint8_t buf[2] = {k_cmd_system, s.raw};
    m_i2c->writeTo(k_i2c_addr, buf, 2);
}