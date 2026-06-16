#include <cstdint>
#include <functional>

#include "ap33772.hpp"
#include "ap33772s.hpp"
#include "event.hpp"
#include "hardware.hpp"
#include "hardware_config.hpp"
#include "ina226.hpp"
#include "pdsink_iface.hpp"
#include "rotary_encoder.hpp"
#include "ssd1306.hpp"
#include "state_machine.hpp"

using hal::gpio::Direction;
using hal::gpio::Edge;
using hal::gpio::Pull;

static constexpr uint k_g_rot_enc_btn_pin = 11;
static constexpr uint k_rot_enc_a_pin = 10;
static constexpr uint k_g_rot_enc_b_pin = 9;

static constexpr i2c_inst_t* k_i2c = i2c1;
static constexpr unsigned int k_i2c_sda_pin = 18;
static constexpr unsigned int k_i2c_scl_pin = 19;
static constexpr unsigned int k_i2c_speed = 400;   // kHz

static constexpr unsigned int k_g_pd_int_pin = 25;
static constexpr unsigned int k_g_output_enable_pin = 17;
static constexpr unsigned int k_g_vout_status_pin = 14;

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

static constexpr PicoGpioPin g_rot_enc_a_pin{k_rot_enc_a_pin};
static constexpr PicoGpioPin g_rot_enc_b_pin{k_g_rot_enc_b_pin};
static constexpr PicoGpioPin g_rot_enc_btn_pin(k_g_rot_enc_btn_pin);
static constexpr PicoGpioPin g_output_enable{k_g_output_enable_pin};
static constexpr PicoGpioPin g_vout_status{k_g_vout_status_pin};
static constexpr PicoGpioPin g_pd_int{k_g_pd_int_pin};
static constexpr PicoI2c g_i2c{k_i2c};
PicoRepeatingTimer g_timer;
RotaryEncoder g_rotary_encoder{g_rot_enc_a_pin, g_rot_enc_b_pin,
                               g_rot_enc_btn_pin};
Ssd1306_128x64 g_oled{g_i2c};
Ina226 g_ina226{g_i2c, k_ina226_addr};
Ap33772 g_ap33772{g_i2c};
Ap33772s g_ap33772s{g_i2c};
std::reference_wrapper<IPdSink> g_pdsink = g_ap33772;

volatile uint32_t g_system_time = 0;
volatile bool g_is_g_pd_interrupt_pending = false;
volatile bool g_is_g_vout_status_interrupt_pending = false;
static std::array<uint8_t, Ssd1306_128x64::getFrameBufferSize()> g_frame_buffer;

auto initialize() -> void {
    g_i2c.initialize(k_i2c_sda_pin, k_i2c_scl_pin, k_i2c_speed);
    g_rotary_encoder.initialize();
    g_output_enable.configure(Direction::Output, Pull::Down);
    g_vout_status.configure(Direction::Input, Pull::Down);
    g_vout_status.attachInterrupt(
        Edge::Falling,
        [](const GpioPin& gpio, void* user) -> void {
            g_is_g_pd_interrupt_pending = true;
        },
        nullptr);
    g_pd_int.configure(Direction::Input, Pull::Down);
    g_pd_int.attachInterrupt(
        Edge::Rising,
        [](const GpioPin& gpio, void* user) -> void {
            g_is_g_pd_interrupt_pending = true;
        },
        nullptr);
    g_pd_int.enableInterrupt(true);
    g_oled.initialize();
    g_ina226.setAveragingMode(Ina226::AveragingMode::Samples128);
    g_ina226.calibrate(5, 0.01);
    Screen::initialize(g_frame_buffer, Ssd1306_128x64::getWidth(),
                       Ssd1306_128x64::getHeight(),
                       Ssd1306_128x64::getPageHeight());
    // By default, use AP33772 if available, otherwise fall back to AP33772S
    if (g_ap33772.probe()) {
        g_ap33772.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        g_ap33772.setOtpThreshold(k_otp_threshold);
        Ap33772::MaskReg mask;
        mask.newpdo_en = 1;
        mask.ocp_en = 1;
        mask.otp_en = 1;
        mask.ovp_en = 1;
        g_ap33772.setMask(mask);
    } else {
        g_pdsink = std::ref(g_ap33772s);
        g_ap33772s.setNtc(k_ntc_tr25, k_ntc_tr50, k_ntc_tr75, k_ntc_tr100);
        g_ap33772s.setOtpThreshold(k_otp_threshold);
        g_ap33772s.setVselMin(k_ap33772s_vsel_min);
        Ap33772s::MaskReg mask;
        mask.newpdo_msk = 1;
        mask.ocp_msk = 1;
        mask.otp_msk = 1;
        mask.ovp_msk = 1;
        mask.uvp_msk = 1;
        g_ap33772s.setMask(mask);
    }
    g_timer.start(
        1, [](void* ctx) -> void { g_system_time = g_system_time + 1; },
        nullptr);
}

auto main() -> int {
    initialize();
    HardwareContext hardware{.pdsink = g_pdsink.get(),
                             .output_enable = g_output_enable,
                             .oled = g_oled};

    StateMachine state_machine{hardware};

    uint32_t last_tick_time = 0;
    uint32_t last_sensor_read_time = 0;

    while (true) {
        uint32_t current_time = g_system_time;
        uint32_t delta = current_time - last_tick_time;
        last_tick_time = current_time;

        g_rotary_encoder.handle(current_time);
        auto encoder_state = g_rotary_encoder.getState();
        if (encoder_state != RotaryEncoder::State::idle &&
            encoder_state != RotaryEncoder::State::processed) {
            state_machine.dispatch(RotaryEncoderEvent{encoder_state});
            g_rotary_encoder.clearState();
        }

        if (current_time - last_sensor_read_time >= k_sensor_read_period) {
            last_sensor_read_time = current_time;
            state_machine.dispatch(SensorUpdateEvent{g_ina226.getBusVoltage(),
                                                     g_ina226.getCurrent(),
                                                     g_pdsink.get().getTemp()});
        }

        if (g_is_g_pd_interrupt_pending) {
            g_is_g_pd_interrupt_pending = false;
            state_machine.dispatch(
                PdSinkStatusUpdateEvent{g_pdsink.get().getStatus()});
        }

        if (g_is_g_vout_status_interrupt_pending) {
            g_is_g_vout_status_interrupt_pending = false;
            state_machine.dispatch(VoutStatusUpdateEvent{g_vout_status.read()});
        }

        state_machine.dispatch(SystemTickEvent{delta});
    }
}
