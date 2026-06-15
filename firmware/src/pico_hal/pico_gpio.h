#ifndef pico_pin_h
#define pico_pin_h

#include "pico/stdlib.h"

#include "gpio_iface.h"

class PicoGpio : public IGpio {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] io_pin RP's IO pin
     */
    PicoGpio(unsigned int io_pin);

    /**
     * Implementation of IGpio interface
     */

    ~PicoGpio() final = default;

    auto configure(Direction dir, Pull pull = Pull::None) const
        -> bool override;

    auto write(bool value) const -> bool override;

    auto read() const -> bool override;

    auto attachInterrupt(Edge edge, IrqCallback callback,
                         void* user = nullptr) const -> bool override;

    auto enableInterrupt(bool enable) const -> void override;

  private:
    unsigned int m_pin;
};

#endif   // pico_pin_h
