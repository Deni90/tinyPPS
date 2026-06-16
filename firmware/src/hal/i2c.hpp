#ifndef i2c_hpp
#define i2c_hpp

#include <concepts>
#include <cstdint>
#include <span>

namespace hal::i2c {
/**
 * @brief Concept I2c data type.
 *
 * @tparam T The type to check.
 *
 * @note This concept requires the type to have `writeTo` and `readFrom` member
 * functions.
 */
template <typename T>
concept I2c =
    requires(const T i2c, uint8_t addr, std::span<const uint8_t> tx_data,
             std::span<uint8_t> rx_data) {
        { i2c.writeTo(addr, tx_data) } -> std::same_as<int>;
        { i2c.readFrom(addr, rx_data) } -> std::same_as<int>;
    };

}   // namespace hal::i2c

#endif   // i2c_hpp
