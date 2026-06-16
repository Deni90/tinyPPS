#ifndef hardware_config_hpp
#define hardware_config_hpp

#include "pico_gpio.hpp"
#include "pico_i2c.hpp"
#include "pico_timer.hpp"

using GpioPin = PicoGpioPin;
using I2c = PicoI2c;
using RepeatingTimer = PicoRepeatingTimer;

#include "ssd1306.hpp"

using Ssd1306_128x64 = Ssd1306<64>;

#endif   // hardware_config_hpp
