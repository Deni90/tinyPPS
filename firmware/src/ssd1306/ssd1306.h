#ifndef ssd1306_h
#define ssd1306_h

#include "i2c_iface.h"

#include <cstdint>

/**
 * @brief Class representing SSD1306 type oled display
 */
class Ssd1306 {
  public:
    /**
     * @brief Enum representing various oled display sizes
     */
    enum class Type { ssd1306_128x32, ssd1306_128x64 };

    /**
     * @brief Constructor
     *
     * @param[in] i2c Pointer to i2c interface implementation
     * @param[in] oled_type Type of the oled display
     */
    Ssd1306(II2c* i2c, Type oled_type);

    /**
     * @brief Initialize the module
     */
    void initialize();

    /**
     * @brief Send a buffer to display.
     *
     * Performs a partial display update using page-level dirty tracking.
     * Instead of pushing the full buffer over I2C, this method only transmits
     * 128-byte segments (pages) that actually contains changes.
     *
     * @param[in] fb Pointer to a buffer containing data meant to be shown
     * on the display
     */
    void display(const uint8_t* fb);

    /**
     * @brief Return screen width
     *
     * @return Screen width in pixels
     */
    uint16_t getWidth() const;

    /**
     * @brief Return screen height
     *
     * @return Screen height in pixels
     */
    uint16_t getHeight() const;

  private:
    /**
     * @brief Send command to display via I2C
     *
     * @param[in] cmd Command
     */
    void sendCommand(uint8_t cmd);

    /**
     * @brief Send multiple commands to display via I2C
     *
     * @param[in] cmds Buffer containing commands
     * @param[in] len Size of the buffer
     */
    void sendCommands(const uint8_t* cmds, uint16_t len);

    II2c* m_i2c;
    Type m_type;
    uint8_t m_old_fb[1024];
};

#endif   // ssd1306_h