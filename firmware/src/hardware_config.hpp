#ifndef hardware_config_hpp
#define hardware_config_hpp

#include "pico_gpio.hpp"
#include "pico_i2c.hpp"
#include "pico_timer.hpp"

using GpioPin = PicoGpioPin;
using I2c = PicoI2c;
using RepeatingTimer = PicoRepeatingTimer;

#endif   // hardware_config_hpp
