#include <cstdint>
#include <functional>

#include "ap33772.h"
#include "ap33772s.h"
#include "event.h"
#include "hardware.h"
#include "ina226.h"
#include "pdsink_iface.h"
#include "pico_gpio.h"
#include "pico_i2c.h"
#include "pico_timer.h"
#include "rotary_encoder.h"
#include "ssd1306.hpp"
#include "state_machine.h"

static constexpr uint k_rot_enc_btn_pin = 11;
static constexpr uint k_rot_enc_a_pin = 10;
static constexpr uint k_rot_enc_b_pin = 9;

static constexpr i2c_inst_t* k_i2c = i2c1;
static constexpr unsigned int k_i2c_sda_pin = 18;
static constexpr unsigned int k_i2c_scl_pin = 19;
static constexpr unsigned int k_i2c_speed = 400;   // kHz

static constexpr unsigned int k_pd_int_pin = 25;
static constexpr unsigned int k_output_enable_pin = 17;
static constexpr unsigned int k_vout_status_pin = 14;

static constexpr uint8_t k_ina226_addr = 0x40;

// https://product.tdk.com/system/files/dam/doc/product/sensor/ntc/chip-ntc-thermistor/data_sheet/datasheet_ntcgs103jx103dt8.pdf
// based on B value:
//                  [at 25/50C] 3380K typ.
//                  [at 25/85C] 3435K+-0.7%
static constexpr unsigned int k_ntc_tr25 = 10000;
static constexpr unsigned int k_ntc_tr50 = 4164;
static constexpr unsigned int k_ntc_tr75 = 1912;
static constexpr unsigned int k_ntc_tr100 = 987;

static constexpr uint16_t k_ap33772s_vsel_min = 3300;
static constexpr uint8_t k_otp_threshold = 85;

static constexpr uint32_t k_sensor_read_period = 20;

volatile uint32_t g_system_time = 0;
volatile bool g_is_pd_interrupt_pending = false;
volatile bool g_is_vout_status_interrupt_pending = false;

auto main() -> int {
    stdio_init_all();

    PicoI2c i2c;
    PicoGpio rot_enc_a_pin{k_rot_enc_a_pin};
    PicoGpio rot_enc_b_pin{k_rot_enc_b_pin};
    PicoGpio rot_enc_btn_pin(k_rot_enc_btn_pin);
    RotaryEncoder rotary_encoder{rot_enc_a_pin, rot_enc_b_pin, rot_enc_btn_pin};
    PicoGpio output_enable{k_output_enable_pin};
    PicoGpio vout_status{k_vout_status_pin};
    PicoGpio pd_int{k_pd_int_pin};
    Ssd1306_128x64 oled{i2c};
    Ina226 ina226{i2c, k_ina226_addr};
    Ap33772 ap33772{i2c};
    Ap33772s ap33772s{i2c};
    PicoRepeatingTimer timer;

    i2c.initialize(k_i2c, k_i2c_sda_pin, k_i2c_scl_pin, k_i2c_speed);
    rotary_encoder.initialize();
    output_enable.configure(IGpio::Direction::Output, IGpio::Pull::Down);
    vout_status.configure(IGpio::Direction::Input, IGpio::Pull::Down);
    vout_status.attachInterrupt(
        IGpio::Edge::Falling,
        [](IGpio& gpio, void* user) -> void {
            g_is_pd_interrupt_pending = true;
        },
        nullptr);
    pd_int.configure(IGpio::Direction::Input, IGpio::Pull::Down);
    pd_int.attachInterrupt(
        IGpio::Edge::Rising,
        [](IGpio& gpio, void* user) -> void {
            g_is_pd_interrupt_pending = true;
        },
        nullptr);
    pd_int.enableInterrupt(true);
    oled.initialize();
    ina226.setAveragingMode(Ina226::AveragingMode::Samples128);
    ina226.calibrate(5, 0.01);
    // By default, use AP33772 if available, otherwise fall back to AP33772S
    std::reference_wrapper<IPdSink> pdsink = ap33772;
    if (ap33772.probe()) {
        ap33772.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        ap33772.setOtpThreshold(k_otp_threshold);
        Ap33772::MaskReg mask;
        mask.newpdo_en = 1;
        mask.ocp_en = 1;
        mask.otp_en = 1;
        mask.ovp_en = 1;
        ap33772.setMask(mask);
    } else {
        pdsink = std::ref(ap33772s);
        ap33772s.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        ap33772s.setOtpThreshold(k_otp_threshold);
        ap33772s.setVselMin(k_ap33772s_vsel_min);
        Ap33772s::MaskReg mask;
        mask.newpdo_msk = 1;
        mask.ocp_msk = 1;
        mask.otp_msk = 1;
        mask.ovp_msk = 1;
        mask.uvp_msk = 1;
        ap33772s.setMask(mask);
    }
    timer.start(
        1, [](void* ctx) -> void { g_system_time = g_system_time + 1; },
        nullptr);
    HardwareContext hardware{.timer = timer,
                             .pdsink = pdsink.get(),
                             .output_enable = output_enable,
                             .vout_status = vout_status,
                             .pd_int = pd_int,
                             .ina226 = ina226,
                             .encoder = rotary_encoder,
                             .oled = oled};

    StateMachine state_machine{hardware};

    uint32_t last_tick_time = 0;
    uint32_t last_sensor_read_time = 0;

    while (true) {
        uint32_t current_time = g_system_time;
        uint32_t delta = current_time - last_tick_time;
        last_tick_time = current_time;

        rotary_encoder.handle(current_time);
        auto encoder_state = rotary_encoder.getState();
        if (encoder_state != RotaryEncoder::State::idle &&
            encoder_state != RotaryEncoder::State::processed) {
            state_machine.dispatch(RotaryEncoderEvent{encoder_state});
            rotary_encoder.clearState();
        }

        if (current_time - last_sensor_read_time >= k_sensor_read_period) {
            last_sensor_read_time = current_time;
            state_machine.dispatch(SensorUpdateEvent{ina226.getBusVoltage(),
                                                     ina226.getCurrent(),
                                                     pdsink.get().getTemp()});
        }

        if (g_is_pd_interrupt_pending) {
            g_is_pd_interrupt_pending = false;
            state_machine.dispatch(
                PdSinkStatusUpdateEvent{pdsink.get().getStatus()});
        }

        if (g_is_vout_status_interrupt_pending) {
            g_is_vout_status_interrupt_pending = false;
            state_machine.dispatch(VoutStatusUpdateEvent{vout_status.read()});
        }

        state_machine.dispatch(SystemTickEvent{delta});
    }
}
