#ifndef hardware_hpp
#define hardware_hpp

#include "hardware_config.hpp"
#include "pdsink_iface.hpp"

struct HardwareContext {
    IPdSink& pdsink;
    const GpioPin& output_enable;
    Ssd1306_128x64& oled;
};

#endif   // hardware_hpp
