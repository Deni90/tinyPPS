#include "pico_gpio.hpp"

#include <cstdint>

#include "gpio.hpp"
#include "pico/stdlib.h"

using hal::gpio::Direction;
using hal::gpio::Edge;
using hal::gpio::IrqCallback;
using hal::gpio::Pull;

/* RP2040 GPIO IRQ demux table */
struct IrqEntry {
    const PicoGpioPin* gpio;
    IrqCallback<PicoGpioPin> callback;
    void* user;
    uint32_t events;
};

static IrqEntry irq_table[NUM_BANK0_GPIOS];
static bool irq_callback_installed = false;

static auto edgeToPico(Edge edge) -> uint32_t {
    switch (edge) {
    case Edge::Rising:
        return GPIO_IRQ_EDGE_RISE;
    case Edge::Falling:
        return GPIO_IRQ_EDGE_FALL;
    case Edge::Both:
        return GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
    default:
        return 0;
    }
}

// Global Pico SDK GPIO IRQ dispatcher
//
// RP2040 supports only one GPIO IRQ callback per core.
// This function demultiplexes interrupts to the correct Gpio instance.
static void gpioIrqDispatch(uint gpio, uint32_t events) {
    if (gpio >= NUM_BANK0_GPIOS) {
        return;
    }

    IrqEntry& entry = irq_table[gpio];

    if (entry.callback && (events & entry.events)) {
        entry.callback(*entry.gpio, entry.user);
    }
}

auto PicoGpioPin::configure(Direction dir, Pull pull) const -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_init(m_pin);
    gpio_set_dir(m_pin, dir == Direction::Output);
    gpio_set_pulls(m_pin, pull == Pull::Up, pull == Pull::Down);
    return true;
}

auto PicoGpioPin::write(bool value) const -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_put(m_pin, value);
    return true;
}

auto PicoGpioPin::read() const -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    return gpio_get(m_pin);
}

auto PicoGpioPin::attachInterrupt(Edge edge, IrqCallback<PicoGpioPin> callback,
                                  void* user) const -> bool {
    if (m_pin >= NUM_BANK0_GPIOS || callback == nullptr) {
        return false;
    }

    IrqEntry& entry = irq_table[m_pin];
    entry.gpio = this;
    entry.callback = callback;
    entry.user = user;
    entry.events = edgeToPico(edge);

    if (!irq_callback_installed) {
        /* First IRQ installs the global dispatcher */
        gpio_set_irq_enabled_with_callback(m_pin, entry.events, true,
                                           &gpioIrqDispatch);
        irq_callback_installed = true;
    } else {
        gpio_set_irq_enabled(m_pin, entry.events, true);
    }

    return true;
}

auto PicoGpioPin::enableInterrupt(bool enable) const -> void {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return;
    }

    gpio_set_irq_enabled(m_pin, irq_table[m_pin].events, enable);
}
