#include <algorithm>

template <uint16_t Height>
auto Ssd1306<Height>::initialize() -> void {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t com_pin_cfg = 0x12;
    if constexpr (Height == 32) {
        com_pin_cfg = 0x02;
    }

    const auto cmds = std::to_array<uint8_t>({
        k_set_disp,   // set display off
        /* memory mapping */
        k_set_mem_mode,   // set memory address mode 0 = horizontal, 1 =
                          // vertical, 2 = page
        0x00,             // horizontal addressing mode
        /* resolution and layout */
        k_disp_start_line,   // set display start line to 0
        k_set_seg_remap |
            0x01,   // set segment re-map, column address 127 is mapped to SEG0
        k_set_mux_ratio,            // set multiplex ratio
        Height - 1,                 // Display height - 1
        k_set_com_out_dir | 0x08,   // set COM (common) output scan direction.
                                    // Scan from bottom up, COM[N-1] to COM0
        k_set_disp_offset,          // set display offset
        0x00,                       // no offset
        k_set_com_pin_cfg,   // set COM (common) pins hardware configuration.
                             // Board specific magic number. 0x02 Works for
                             // 128x32, 0x12 Possibly works for 128x64. Other
                             // options 0x22, 0x32
        com_pin_cfg,
        /* timing and driving scheme */
        k_set_disp_clk_div,   // set display clock divide ratio
        0x80,                 // div ratio of 1, standard freq
        k_set_precharge,      // set pre-charge period
        0xF1,                 // Vcc internally generated on our board
        k_set_vcom_desel,     // set VCOMH deselect level
        0x30,                 // 0.83xVcc
        /* display */
        k_set_contrast,   // set contrast control
        200,
        k_set_entire_on,     // set entire display on to follow RAM content
        k_set_norm_disp,     // set normal (not inverted) display
        k_set_charge_pump,   // set charge pump
        0x14,                // Vcc internally generated on our board
        k_set_scroll |
            0x00,   // deactivate horizontal scrolling if set. This is necessary
                    // as memory writes will corrupt if scrolling was enabled
        k_set_disp | 0x01,   // turn display on
    });
    sendCommands(cmds);
}

template <uint16_t Height>
auto Ssd1306<Height>::display(std::span<const uint8_t> frame_buffer) -> void {
    if (m_old_fb.size() != frame_buffer.size()) {
        return;
    }
    for (uint8_t page = 0; page < k_page_height; page++) {
        const auto offset = page * k_width;
        const auto old_fb_slice = std::span{m_old_fb}.subspan(offset, k_width);
        if (std::ranges::equal(old_fb_slice,
                               frame_buffer.subspan(offset, k_width))) {
            // new page is same as old, no update required
            continue;
        }
        // Update only dirty pages
        const auto cmds = std::to_array<uint8_t>({
            0x00,
            static_cast<uint8_t>(0xB0 |
                                 page),   // Set target page (0xB0 to 0xB7)
            0x00,                         // Set lower column start (0)
            0x10                          // Set higher column start (0)
        });
        m_i2c.writeTo(k_i2c_addr, cmds);
        std::array<uint8_t, k_width + 1> temp_buf;
        temp_buf[0] = 0x40;
        std::ranges::copy(frame_buffer.subspan(offset, k_width),
                          temp_buf.begin() + 1);
        m_i2c.writeTo(k_i2c_addr, temp_buf);
    }
    std::ranges::copy(frame_buffer, m_old_fb.begin());
}

template <uint16_t Height>
auto Ssd1306<Height>::sendCommand(uint8_t cmd) -> void {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    std::array<uint8_t, 2> buf = {0x80, cmd};
    m_i2c.writeTo(k_i2c_addr, buf);
}

template <uint16_t Height>
auto Ssd1306<Height>::sendCommands(std::span<const uint8_t> cmds) -> void {
    for (const auto& cmd : cmds) {
        sendCommand(cmd);
    }
}
