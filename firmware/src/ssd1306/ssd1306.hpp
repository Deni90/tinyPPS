#ifndef ssd1306_h
#define ssd1306_h

#include "i2c_iface.h"

#include <array>
#include <cstdint>
#include <span>

/**
 * @brief Class representing SSD1306 type oled display
 */
template <uint16_t Height>
class Ssd1306 {
    static_assert(Height == 32 || Height == 64,
                  "Unsupported oled display height");

  public:
    /**
     * @brief Constructor
     *
     * @param[in] i2c Reference to i2c interface implementation
     */
    explicit Ssd1306(II2c& i2c) : m_i2c(i2c) {}

    /**
     * @brief Initialize the module
     */
    auto initialize() -> void;

    /**
     * @brief Send a buffer to display.
     *
     * Performs a partial display update using page-level dirty tracking.
     * Instead of pushing the full buffer over I2C, this method only transmits
     * 128-byte segments (pages) that actually contains changes.
     *
     * @param[in] frame_buffer A constant view of the contiguous image or pixel
     * data.
     */
    auto display(std::span<const uint8_t> frame_buffer) -> void;

    /**
     * @brief Return screen width
     *
     * @return Screen width in pixels
     */
    [[nodiscard]] static constexpr auto getWidth() -> uint16_t {
        return k_width;
    }

    /**
     * @brief Return screen height
     *
     * @return Screen height in pixels
     */
    [[nodiscard]] static constexpr auto getHeight() -> uint16_t {
        return Height;
    }

    /**
     * @brief Return the height of a page in pixels
     *
     * @return Page height in pixels
     */
    [[nodiscard]] static constexpr auto getPageHeight() -> uint16_t {
        return Height / 8;
    }

    /**
     * @brief Return the size of the frame buffer in bytes
     *
     * @return Frame buffer size in bytes
     */
    [[nodiscard]] static constexpr auto getFrameBufferSize() -> uint16_t {
        return k_width * Height / 8;
    }

  private:
    static constexpr uint16_t k_width = 128;
    static constexpr uint16_t k_page_height = Height / 8;
    // SSD1306 commands
    static constexpr uint8_t k_i2c_addr = 0x3C;
    static constexpr uint8_t k_set_mem_mode = 0x20;
    static constexpr uint8_t k_col_addr = 0x21;
    static constexpr uint8_t k_page_addr = 0x22;
    static constexpr uint8_t k_horiz_scroll = 0x26;
    static constexpr uint8_t k_set_scroll = 0x2E;
    static constexpr uint8_t k_disp_start_line = 0x40;
    static constexpr uint8_t k_set_contrast = 0x81;
    static constexpr uint8_t k_set_charge_pump = 0x8D;
    static constexpr uint8_t k_set_seg_remap = 0xA0;
    static constexpr uint8_t k_set_entire_on = 0xA4;
    static constexpr uint8_t k_set_all_on = 0xA5;
    static constexpr uint8_t k_set_norm_disp = 0xA6;
    static constexpr uint8_t k_set_inv_disp = 0xA7;
    static constexpr uint8_t k_set_mux_ratio = 0xA8;
    static constexpr uint8_t k_set_disp = 0xAE;
    static constexpr uint8_t k_set_com_out_dir = 0xC0;
    static constexpr uint8_t k_set_com_out_dir_flip = 0xC0;
    static constexpr uint8_t k_set_disp_offset = 0xD3;
    static constexpr uint8_t k_set_disp_clk_div = 0xD5;
    static constexpr uint8_t k_set_precharge = 0xD9;
    static constexpr uint8_t k_set_com_pin_cfg = 0xDA;
    static constexpr uint8_t k_set_vcom_desel = 0xDB;

    /**
     * @brief Send command to display via I2C
     *
     * @param[in] cmd Command
     */
    auto sendCommand(uint8_t cmd) -> void;

    /**
     * @brief Send multiple commands to display via I2C
     *
     * @param[in] constant view of the command sequence buffer to transmit.
     */
    auto sendCommands(std::span<const uint8_t> cmds) -> void;

    II2c& m_i2c;
    std::array<uint8_t, k_width * k_page_height> m_old_fb;
};

#include "ssd1306.inl"

#endif   // ssd1306_h
