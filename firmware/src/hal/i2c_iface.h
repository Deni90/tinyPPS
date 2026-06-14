#ifndef i2c_iface_h
#define i2c_iface_h

#include <cstdint>
#include <span>

class II2c {
  public:
    /**
     * @brief Destructor for the interface.
     */
    virtual ~II2c() = default;

    /**
     *  @brief Attempt to write specified number of bytes to address
     *
     * @param addr 7-bit address of device to write to
     * @param data A constant view of the data buffer to be sent
     * @return Number of bytes written, or error
     */
    virtual auto writeTo(uint8_t addr, std::span<const uint8_t> data)
        -> int = 0;

    /**
     * @brief Attempt to read specified number of bytes from address
     *
     * @param addr 7-bit address of device to read from
     * @param data A mutable view of the destination buffer where data will be
     * stored.
     * @return Number of bytes read, or error
     */
    virtual auto readFrom(uint8_t addr, std::span<uint8_t> data) -> int = 0;
};

#endif   // i2c_iface_h
