#include "screen.h"

#include <cstring>

static constexpr uint8_t k_font_width = 5;
static constexpr uint8_t k_font_height = 8;
static constexpr uint8_t k_unused = 0x80;
// This is the font we use with the function print* functions
// Each character has 5 pixels avaliable in width, but it doesn't have to use
// all 5. If a character uses less than 5 pixels in width, unused pixels are
// filled with 0x80.
static const uint8_t k_font[] = {
    0x00, 0x00,     0x00,     k_unused, k_unused,   // space
    0x5c, k_unused, k_unused, k_unused, k_unused,   // !
    0x0c, 0x00,     0x0c,     k_unused, k_unused,   // "
    0x28, 0x7c,     0x28,     0x7c,     0x28,       // #
    0x50, 0x58,     0xec,     0x28,     k_unused,   // $
    0x04, 0x60,     0x10,     0x0c,     0x40,       // %
    0x28, 0x54,     0x54,     0x20,     0x50,       // &
    0x0c, k_unused, k_unused, k_unused, k_unused,   // '
    0x38, 0x44,     k_unused, k_unused, k_unused,   // (
    0x44, 0x38,     k_unused, k_unused, k_unused,   // )
    0x14, 0x08,     0x14,     k_unused, k_unused,   //  *
    0x10, 0x38,     0x10,     k_unused, k_unused,   // +
    0xc0, 0x40,     k_unused, k_unused, k_unused,   // ,
    0x10, 0x10,     0x10,     k_unused, k_unused,   // -
    0x40, k_unused, k_unused, k_unused, k_unused,   // .
    0x40, 0x20,     0x10,     0x08,     0x04,       // /
    0x38, 0x44,     0x44,     0x38,     k_unused,   // 0
    0x00, 0x04,     0x7c,     0x00,     k_unused,   // 1
    0x64, 0x54,     0x54,     0x48,     k_unused,   // 2
    0x44, 0x54,     0x54,     0x28,     k_unused,   // 3
    0x30, 0x28,     0x7c,     0x20,     k_unused,   // 4
    0x5c, 0x54,     0x54,     0x24,     k_unused,   // 5
    0x38, 0x54,     0x54,     0x20,     k_unused,   // 6
    0x04, 0x64,     0x14,     0x0c,     k_unused,   // 7
    0x28, 0x54,     0x54,     0x28,     k_unused,   // 8
    0x08, 0x54,     0x54,     0x38,     k_unused,   // 9
    0x00, 0x28,     0x00,     k_unused, k_unused,   // :
    0x68, k_unused, k_unused, k_unused, k_unused,   // ;
    0x10, 0x28,     0x44,     k_unused, k_unused,   // <
    0x28, 0x28,     0x28,     k_unused, k_unused,   // =
    0x44, 0x28,     0x10,     k_unused, k_unused,   // >
    0x04, 0x54,     0x14,     0x08,     k_unused,   // ?
    0x38, 0x44,     0x74,     0x54,     0x38,       // @
    0x78, 0x24,     0x24,     0x78,     k_unused,   // A
    0x7c, 0x54,     0x54,     0x28,     k_unused,   // B
    0x38, 0x44,     0x44,     k_unused, k_unused,   // C
    0x7c, 0x44,     0x44,     0x38,     k_unused,   // D
    0x7c, 0x54,     0x54,     0x44,     k_unused,   // E
    0x7c, 0x14,     0x14,     0x04,     k_unused,   // F
    0x38, 0x44,     0x54,     0x74,     k_unused,   // G
    0x7c, 0x10,     0x10,     0x7c,     k_unused,   // H
    0x44, 0x7c,     0x44,     k_unused, k_unused,   // I
    0x20, 0x40,     0x44,     0x3c,     k_unused,   // J
    0x7c, 0x10,     0x28,     0x44,     k_unused,   // K
    0x7c, 0x40,     0x40,     k_unused, k_unused,   // L
    0x7c, 0x08,     0x10,     0x08,     0x7c,       // M
    0x7c, 0x08,     0x10,     0x7c,     k_unused,   // N
    0x38, 0x44,     0x44,     0x38,     k_unused,   // O
    0x7c, 0x24,     0x24,     0x18,     k_unused,   // P
    0x38, 0x44,     0x44,     0xb8,     k_unused,   // Q
    0x7c, 0x24,     0x24,     0x58,     k_unused,   // R
    0x48, 0x54,     0x54,     0x24,     k_unused,   // S
    0x04, 0x7c,     0x04,     k_unused, k_unused,   // T
    0x3c, 0x40,     0x40,     0x3c,     k_unused,   // U
    0x3c, 0x40,     0x30,     0x0c,     k_unused,   // V
    0x3c, 0x40,     0x38,     0x40,     0x3c,       // W
    0x6c, 0x10,     0x10,     0x6c,     k_unused,   // X
    0x0c, 0x50,     0x50,     0x3c,     k_unused,   // Y
    0x64, 0x54,     0x4c,     k_unused, k_unused,   // Z
    0x7c, 0x44,     k_unused, k_unused, k_unused,   // [
    0x04, 0x08,     0x10,     0x20,     0x40,       /* \ */
    0x44, 0x7c,     k_unused, k_unused, k_unused,   // ]
    0x08, 0x04,     0x08,     k_unused, k_unused,   // ^
    0x40, 0x40,     0x40,     0x40,     k_unused,   // _
    0x04, 0x08,     k_unused, k_unused, k_unused,   // `
    0x30, 0x48,     0x48,     0x78,     k_unused,   // a
    0x7c, 0x48,     0x48,     0x30,     k_unused,   // b
    0x30, 0x48,     0x48,     k_unused, k_unused,   // c
    0x30, 0x48,     0x48,     0x7c,     k_unused,   // d
    0x30, 0x68,     0x58,     0x10,     k_unused,   // e
    0x10, 0x78,     0x14,     k_unused, k_unused,   // f
    0x18, 0xa4,     0xa4,     0x7c,     k_unused,   // g
    0x7c, 0x08,     0x08,     0x70,     k_unused,   // h
    0x74, k_unused, k_unused, k_unused, k_unused,   // i
    0x40, 0x34,     k_unused, k_unused, k_unused,   // j
    0x7c, 0x20,     0x30,     0x48,     k_unused,   // k
    0x7c, k_unused, k_unused, k_unused, k_unused,   // l
    0x78, 0x08,     0x78,     0x08,     0x70,       // m
    0x78, 0x08,     0x08,     0x70,     k_unused,   // n
    0x30, 0x48,     0x48,     0x30,     k_unused,   // o
    0xf8, 0x48,     0x48,     0x30,     k_unused,   // p
    0x30, 0x48,     0x48,     0xf8,     k_unused,   // q
    0x78, 0x10,     0x08,     k_unused, k_unused,   // r
    0x50, 0x58,     0x68,     0x28,     k_unused,   // s
    0x08, 0x3c,     0x48,     k_unused, k_unused,   // t
    0x38, 0x40,     0x40,     0x78,     k_unused,   // u
    0x38, 0x40,     0x20,     0x18,     k_unused,   // v
    0x18, 0x60,     0x18,     0x60,     0x18,       // w
    0x48, 0x30,     0x48,     k_unused, k_unused,   // x
    0x18, 0xa0,     0xa0,     0x78,     k_unused,   // y
    0x48, 0x68,     0x58,     0x48,     k_unused,   // z
    0x10, 0x6c,     0x44,     k_unused, k_unused,   // {
    0x7e, k_unused, k_unused, k_unused, k_unused,   // |
    0x44, 0x6c,     0x10,     k_unused, k_unused,   // }
    0x08, 0x04,     0x08,     0x04,     k_unused,   // ~
};

Screen::Screen(uint16_t width, uint16_t height)
    : m_width(width), m_height(height) {
    m_frame_buffer = new uint8_t[width * height / 8];
    clear();
}

Screen::~Screen() { delete[] m_frame_buffer; }

void Screen::clear() {
    memset(m_frame_buffer, 0, sizeof(uint8_t) * (m_width * m_height / 8));
}

void Screen::setPixel(int16_t x, int16_t y) {
    if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) {
        return;
    }
    auto buffer_index = m_width * (y / 8) + x;
    m_frame_buffer[buffer_index] |= (1 << (y % 8));
}

void Screen::drawRectangle(int16_t x, int16_t y, uint16_t width,
                           uint16_t height, bool fill) {
    auto xx = x + width;
    auto yy = y + height;
    for (auto i = y; i < yy; i++) {
        for (auto j = x; j < xx; j++) {
            if (i < 0 || j < 0) {
                continue;
            }
            if (fill) {
                setPixel(j, i);
            } else {
                if (i == y || i == (yy - 1) || j == x || j == (xx - 1)) {
                    setPixel(j, i);
                }
            }
        }
    }
}

void Screen::draw(int16_t x, int16_t y, const uint8_t* object, uint16_t width,
                  uint16_t height, bool invert) {
    for (auto yy = y; yy < (y + height); yy++) {
        for (auto xx = x; xx < (x + width); xx++) {
            if ((xx < 0) || (yy < 0) || (xx >= m_width) || (yy >= m_height)) {
                continue;
            }
            auto buffer_index = m_width * (yy / 8) + xx;
            auto object_x = xx - x;
            auto object_y = yy - y;
            auto object_index = width * (object_y / 8) + object_x;
            auto tmp_object = object[object_index];
            if (invert) {
                tmp_object = ~tmp_object;
            }
            if ((tmp_object >> (object_y % 8) & 0x01) == 0x01) {
                m_frame_buffer[buffer_index] |= (1 << (yy % 8));
            } else {
                m_frame_buffer[buffer_index] &= ~(1 << (yy % 8));
            }
        }
    }
}

uint16_t Screen::printChar(int16_t x, int16_t y, char ch, bool invert) {
    auto char_width = 0;
    for (auto i = 0; i < k_font_width; i++) {
        if (k_font[(ch - ' ') * 5 + i] != k_unused) {
            ++char_width;
        }
    }
    draw(x, y, &k_font[(ch - ' ') * k_font_width], char_width, k_font_height,
         invert);   // draw the character
    draw(x + char_width, y, 0x00, 1, k_font_height,
         invert);            // add letter spacing
    return char_width + 1;   // +1 is for letter spacing
}

uint16_t Screen::printCharBig(int16_t x, int16_t y, char ch, bool invert) {
    uint8_t font_element, big_font_element = 0x00, temp_col, temp_row;
    uint8_t char_width = 0;
    for (auto j = 0; j < k_font_width * 2; j++) {
        font_element = k_font[(ch - ' ') * 5 + j / 2];
        // skip empty space
        if (font_element == k_unused) {
            continue;
        }
        ++char_width;
        // interpolate the upper part of the character element
        for (auto i = 0; i < 8; i++) {
            if ((1 << i / 2) & font_element) {
                big_font_element |= (1 << i);
            } else {
                big_font_element &= ~(1 << i);
            }
        }
        draw(x + j, y, &big_font_element, 1, 8, invert);
        // interpolate the bottom part of the character element
        for (auto i = 0; i < 8; i++) {
            if ((1 << (4 + i / 2)) & font_element)
                big_font_element |= (1 << i);
            else
                big_font_element &= ~(1 << i);
        }
        draw(x + j, y + 8, &big_font_element, 1, 8, invert);
    }
    return char_width + 2;   // +2 is for letter spacing
}

uint16_t Screen::printString(int16_t x, int16_t y, const std::string& str) {
    return printString(x, y, str, StringConfig());
}

uint16_t Screen::printString(int16_t x, int16_t y, const std::string& str,
                             const StringConfig& config) {
    auto text_width = 0;
    if (config.align == TextAlign::center || config.align == TextAlign::right) {
        // dry run to calculate width used for alignment
        for (const auto& ch : str) {
            for (uint8_t i = 0; i < k_font_width; i++) {
                if (k_font[(ch - ' ') * k_font_width + i] != k_unused) {
                    ++text_width;
                }
            }
        }
        text_width += str.size() - 1;   // add letter spacing to text width
        if (config.size == FontSize::big) {
            text_width *= 2;
        }
    }
    int32_t xx = x;
    if (config.align == TextAlign::center) {
        xx = x - text_width / 2;
    } else if (config.align == TextAlign::right) {
        xx = x - text_width;
    }
    for (const auto& ch : str) {
        if (config.size == FontSize::big) {
            xx += printCharBig(xx, y, ch, config.invert);
        } else {
            xx += printChar(xx, y, ch, config.invert);
        }
    }
    return xx - x;
}