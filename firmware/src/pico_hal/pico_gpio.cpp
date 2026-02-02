#include "pico_gpio.h"

/* RP2040 GPIO IRQ demux table */
struct IrqEntry {
    IGpio* gpio;
    IGpio::IrqCallback cb;
    void* user;
    uint32_t events;
};

static IrqEntry irq_table[NUM_BANK0_GPIOS];
static bool irq_callback_installed = false;

static uint32_t edgeToPico(IGpio::Edge edge) {
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
    if (gpio >= NUM_BANK0_GPIOS)
        return;

    IrqEntry& e = irq_table[gpio];

    if (e.cb && (events & e.events)) {
        e.cb(*e.gpio, e.user);
    }
}

PicoGpio::PicoGpio(unsigned int io_pin) : m_pin(io_pin) { gpio_init(m_pin); }

bool PicoGpio::configure(Direction dir, Pull pull) {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_set_dir(m_pin, dir == Direction::Output);
    gpio_set_pulls(m_pin, pull == Pull::Up, pull == Pull::Down);
    return true;
}

bool PicoGpio::write(bool value) {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    gpio_put(m_pin, value);
    return true;
}

bool PicoGpio::read() {
    if (m_pin >= NUM_BANK0_GPIOS) {
        return false;
    }
    return gpio_get(m_pin);
}

bool PicoGpio::attachInterrupt(Edge edge, IrqCallback cb, void* user) {
    if (m_pin >= NUM_BANK0_GPIOS || cb == nullptr)
        return false;

    IrqEntry& e = irq_table[m_pin];
    e.gpio = this;
    e.cb = cb;
    e.user = user;
    e.events = edgeToPico(edge);

    if (!irq_callback_installed) {
        /* First IRQ installs the global dispatcher */
        gpio_set_irq_enabled_with_callback(m_pin, e.events, true,
                                           &gpioIrqDispatch);
        irq_callback_installed = true;
    } else {
        gpio_set_irq_enabled(m_pin, e.events, true);
    }

    return true;
}

void PicoGpio::enableInterrupt(bool enable) {
    if (m_pin >= NUM_BANK0_GPIOS)
        return;

    gpio_set_irq_enabled(m_pin, irq_table[m_pin].events, enable);
}