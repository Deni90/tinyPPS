#include "menu_screen.hpp"

#include "pdo_helper.hpp"
#include "screen.hpp"
#include "tiny_format.hpp"

MenuScreen::MenuScreen(std::string_view title) : m_title(title) {}

auto MenuScreen::getTitle() const -> std::string_view { return m_title; }

auto MenuScreen::setConfig(std::span<const Config> config) -> MenuScreen& {
    m_config = config;
    return *this;
}

auto MenuScreen::getSelectedMenuItem() const -> uint8_t {
    return m_selected_menu_item;
}

auto MenuScreen::selectMenuItem(uint8_t index) -> MenuScreen& {
    m_selected_menu_item = index;
    return *this;
}

auto MenuScreen::selectNextMenuItem() -> MenuScreen& {
    m_selected_menu_item = (m_selected_menu_item + 1) % m_config.size();
    return *this;
}

auto MenuScreen::selectPreviousMenuItem() -> MenuScreen& {
    m_selected_menu_item =
        (m_selected_menu_item - 1 + m_config.size()) % m_config.size();
    return *this;
}

auto MenuScreen::build() -> FrameBuffer& {
    clear();
    if (m_config.empty()) {
        return m_frame_buffer;
    }
    uint16_t y_pos = 0;
    std::array<char, 8> buffer;
    printString(
        0, y_pos,
        tinyFormat(buffer, "%d/%d", m_selected_menu_item + 1, m_config.size()));
    printString(m_width / 2, y_pos, m_title, {.align = TextAlign::center});

    y_pos += 2 * m_page_height;
    auto max_menu_size = (m_height / m_page_height) - 2;

    std::size_t start_index =
        (m_selected_menu_item / max_menu_size) * max_menu_size;
    for (auto i = 0; i < max_menu_size; ++i) {
        if ((start_index + i) >= m_config.size()) {
            break;
        }
        bool is_selected = false;
        if (start_index + i == m_selected_menu_item) {
            is_selected = true;
            drawRectangle(0, y_pos, 1, m_page_height, false);
        }
        std::array<char, 24> buffer;
        printString(1, y_pos,
                    pdoToString(m_config[start_index + i].pdo, buffer),
                    {.invert = is_selected});
        y_pos += m_page_height;
    }
    return m_frame_buffer;
}
