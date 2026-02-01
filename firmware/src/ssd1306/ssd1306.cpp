#include "ssd1306.h"

#include <cstring>

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

// Other helper constants
static constexpr uint8_t k_width = 128;
static constexpr uint8_t k_page_height = 8;

Ssd1306::Ssd1306(II2c* i2c, Ssd1306::Type oled_type)
    : m_i2c(i2c), m_type(oled_type) {}

void Ssd1306::initialize() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t com_pin_cfg = 0x12;
    uint8_t display_height = 64;
    if (m_type == Type::ssd1306_128x32) {
        com_pin_cfg = 0x02;
        display_height = 32;
    }

    uint8_t cmds[] = {
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
        --display_height,           // Display height - 1
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
        0xFF,
        k_set_entire_on,     // set entire display on to follow RAM content
        k_set_norm_disp,     // set normal (not inverted) display
        k_set_charge_pump,   // set charge pump
        0x14,                // Vcc internally generated on our board
        k_set_scroll |
            0x00,   // deactivate horizontal scrolling if set. This is necessary
                    // as memory writes will corrupt if scrolling was enabled
        k_set_disp | 0x01,   // turn display on
    };
    sendCommands(cmds, sizeof(cmds));
}

void Ssd1306::display(const uint8_t* fb) {
    const uint8_t pages = (m_type == Type::ssd1306_128x64) ? 8 : 4;
    for (uint8_t page = 0; page < pages; page++) {
        int base = page * k_width;
        if (memcmp(fb + base, m_old_fb + base, k_width) == 0) {
            // new page is same as old, no update required
            continue;
        }
        // Update only dirty pages
        uint8_t cmds[] = {
            0x00,
            static_cast<uint8_t>(0xB0 |
                                 page),   // Set target page (0xB0 to 0xB7)
            0x00,                         // Set lower column start (0)
            0x10                          // Set higher column start (0)
        };
        m_i2c->writeTo(k_i2c_addr, cmds, sizeof(cmds));
        uint8_t temp_buf[k_width + 1];
        temp_buf[0] = 0x40;
        memcpy(temp_buf + 1, fb + base, k_width);
        m_i2c->writeTo(k_i2c_addr, temp_buf, sizeof(temp_buf));
    }
    memcpy(m_old_fb, fb, pages * k_width);
}

uint16_t Ssd1306::getWidth() const { return k_width; }

uint16_t Ssd1306::getHeight() const {
    return m_type == Type::ssd1306_128x64 ? 64 : 32;
}

void Ssd1306::sendCommand(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    m_i2c->writeTo(k_i2c_addr, buf, 2);
}

void Ssd1306::sendCommands(const uint8_t* cmds, uint16_t len) {
    for (int i = 0; i < len; ++i) {
        sendCommand(cmds[i]);
    }
}