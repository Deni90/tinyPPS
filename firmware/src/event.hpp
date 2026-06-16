#ifndef event_hpp
#define event_hpp

#include <variant>

#include "pdsink_iface.hpp"
#include "rotary_encoder.hpp"

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

/**
 * @brief Event type for PD sink status update events.
 */
struct PdSinkStatusUpdateEvent {
    IPdSink::Status status;
};

/**
 * @brief Event type for VOUT status update events.
 */
struct VoutStatusUpdateEvent {
    bool enabled{false};
};

/**
 * @brief System event variant that holds one of the supported event types.
 */
using SystemEvent =
    std::variant<RotaryEncoderEvent, SensorUpdateEvent, SystemTickEvent,
                 PdSinkStatusUpdateEvent, VoutStatusUpdateEvent>;

#endif   // event_hpp
