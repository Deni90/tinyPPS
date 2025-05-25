#ifndef i2c_iface_h
#define i2c_iface_h

#include <cstdint>

class II2c {
  public:
    /**
     *  @brief Attempt to write specified number of bytes to address
     *
     * @param addr 7-bit address of device to write to
     * @param data Pointer to data to send
     * @param len Length of data in bytes to send
     * @return Number of bytes written, or error
     */
    virtual int writeTo(uint8_t addr, const uint8_t* data,
                        unsigned int len) = 0;

    /**
     * @brief Attempt to read specified number of bytes from address
     *
     * @param addr 7-bit address of device to read from
     * @param data Pointer to receive data
     * @param len Length of data in bytes to receive
     * @return Number of bytes read, or error
     */
    virtual int readFrom(uint8_t addr, uint8_t* data, unsigned int len) = 0;
};

#endif   // i2c_iface_h