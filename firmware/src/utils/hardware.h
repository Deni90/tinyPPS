#ifndef hardware_h
#define hardware_h

#include "hardware_config.hpp"
#include "ina226.h"
#include "pdsink_iface.h"
#include "rotary_encoder.h"
#include "ssd1306.hpp"
#include "timer_iface.h"

using Ssd1306_128x64 = Ssd1306<64>;

struct HardwareContext {
    IRepeatingTimer& timer;
    IPdSink& pdsink;
    const GpioPin& output_enable;
    const GpioPin& vout_status;
    const GpioPin& pd_int;
    Ina226& ina226;
    RotaryEncoder& encoder;
    Ssd1306_128x64& oled;
};

#endif   // hardware_h
