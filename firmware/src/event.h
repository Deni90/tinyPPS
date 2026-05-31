#ifndef event_h
#define event_h

#include <variant>

#include "pdsink_iface.h"
#include "rotary_encoder.h"

/**
 * @brief Event type for rotary encoder state changes.
 */
struct RotaryEncoderEvent {
    RotaryEncoder::State encoder_state{RotaryEncoder::State::idle};
};

/**
 * @brief Event type for INA226 update events.
 */
struct SensorUpdateEvent {
    float voltage{0.0F};
    float current{0.0F};
    uint8_t temperature{0};
};

/**
 * @brief Event type for system tick events.
 */
struct SystemTickEvent {
    uint32_t delta{0};
};

struct PdSinkStatusUpdateEvent {
    IPdSink::Status status;
};

/**
 * @brief System event variant that holds one of the supported event types.
 */
using SystemEvent = std::variant<RotaryEncoderEvent, SensorUpdateEvent,
                                 SystemTickEvent, PdSinkStatusUpdateEvent>;

#endif   // event_h
