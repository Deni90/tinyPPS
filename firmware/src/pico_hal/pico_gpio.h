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

    virtual ~PicoGpio() = default;

    bool configure(Direction dir, Pull pull = Pull::None);

    bool write(bool value) override;

    bool read() override;

  private:
    unsigned int m_pin;
};

#endif   // pico_pin_h