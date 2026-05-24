#include "pico_gpio.h"

/* RP2040 GPIO IRQ demux table */
struct IrqEntry {
    IGpio* gpio;
    IGpio::IrqCallback callback;
    void* user;
    uint32_t events;
};

static IrqEntry irq_table[NUM_BANK0_GPIOS];
static bool irq_callback_installed = false;

static auto edgeToPico(IGpio::Edge edge) -> uint32_t {
    switch (edge) {
    case IGpio::Edge::Rising:
        return GPIO_IRQ_EDGE_RISE;
    case IGpio::Edge::Falling:
        return GPIO_IRQ_EDGE_FALL;
    case IGpio::Edge::Both:
        return GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
    default:
        return 0;
    }
}

/**
 * @brief Global Pico SDK GPIO IRQ dispatcher
 *
 * RP2040 supports only one GPIO IRQ callback per core.
 * This function demultiplexes interrupts to the correct Gpio instance.
 */
static void gpioIrqDispatch(uint gpio, uint32_t events) {
    if (gpio >= NUM_BANK0_GPIOS) {
        return;
    }

    IrqEntry& entry = irq_table[gpio];

    if (entry.callback && (events & entry.events)) {
        entry.callback(*entry.gpio, entry.user);
    }
}

PicoGpio::PicoGpio(unsigned int io_pin) : m_pin(io_pin) { gpio_init(m_pin); }

auto PicoGpio::configure(Direction dir, Pull pull) -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_set_dir(m_pin, dir == Direction::Output);
    gpio_set_pulls(m_pin, pull == Pull::Up, pull == Pull::Down);
    return true;
}

auto PicoGpio::write(bool value) -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_put(m_pin, value);
    return true;
}

auto PicoGpio::read() -> bool {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    return gpio_get(m_pin);
}

auto PicoGpio::attachInterrupt(Edge edge, IrqCallback callback, void* user)
    -> bool {
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

auto PicoGpio::enableInterrupt(bool enable) -> void {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return;
    }

    gpio_set_irq_enabled(m_pin, irq_table[m_pin].events, enable);
}
