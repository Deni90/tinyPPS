#include "menu_screen.h"

#include <format>

static constexpr uint8_t k_page_height = 8;

MenuScreen::MenuScreen(uint16_t width, uint16_t height)
    : Screen(width, height) {}

auto MenuScreen::setTitle(const std::string& title) -> MenuScreen& {
    m_title = title;
    return *this;
}

auto MenuScreen::getTitle() const -> std::string { return m_title; }

auto MenuScreen::setMenuItems(const std::vector<std::string>& menu_items)
    -> MenuScreen& {
    m_menu_items = menu_items;
    return *this;
}

auto MenuScreen::getMenuItems() const -> std::vector<std::string> {
    return m_menu_items;
}

auto MenuScreen::getSelectedMenuItem() const -> uint8_t {
    return m_selected_menu_item;
}

auto MenuScreen::selectMenuItem(uint8_t index) -> MenuScreen& {
    m_selected_menu_item = index;
    return *this;
}

auto MenuScreen::build() -> uint8_t* {
    clear();
    uint16_t y_pos = 0;
    printString(
        0, y_pos,
        std::format("{}/{}", m_selected_menu_item + 1, m_menu_items.size()));
    printString(m_width / 2, y_pos, m_title, {.align = TextAlign::center});

    y_pos += 2 * k_page_height;
    auto max_menu_size = (m_height / k_page_height) - 2;

    auto start_index = (m_selected_menu_item / max_menu_size) * max_menu_size;
    for (auto i = 0; i < max_menu_size; ++i) {
        if ((start_index + i) >= m_menu_items.size()) {
            break;
        }
        bool is_selected = false;
        if (start_index + i == m_selected_menu_item) {
            is_selected = true;
            drawRectangle(0, y_pos, 1, k_page_height, false);
        }
        printString(1, y_pos, m_menu_items[start_index + i],
                    {.invert = is_selected});
        y_pos += k_page_height;
    }
    return m_frame_buffer;
}
