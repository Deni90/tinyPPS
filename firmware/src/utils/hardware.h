#ifndef hardware_h
#define hardware_h

#include "gpio_iface.h"
#include "ina226.h"
#include "pdsink_iface.h"
#include "rotary_encoder.h"
#include "ssd1306.h"
#include "timer_iface.h"

struct HardwareContext {
    IRepeatingTimer& timer;
    IPdSink& pdsink;
    IGpio& output_enable;
    IGpio& vout_status;
    IGpio& pd_int;
    Ina226& ina226;
    RotaryEncoder& encoder;
    Ssd1306& oled;
};

#endif   // hardware_h
