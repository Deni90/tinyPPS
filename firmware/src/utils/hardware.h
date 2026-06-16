#ifndef hardware_h
#define hardware_h

#include "hardware_config.hpp"
#include "pdsink_iface.h"

struct HardwareContext {
    IPdSink& pdsink;
    const GpioPin& output_enable;
    Ssd1306_128x64& oled;
};

#endif   // hardware_h
