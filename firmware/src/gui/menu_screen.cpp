#include "menu_screen.h"

#include <format>

MenuScreen::MenuScreen(uint16_t width, uint16_t height)
    : Screen(width, height), m_selected_menu_item(0) {}

MenuScreen& MenuScreen::setTitle(const std::string& title) {
    m_title = title;
    return *this;
}

std::string MenuScreen::getTitle() const { return m_title; }

MenuScreen&
MenuScreen::setMenuItems(const std::vector<std::string>& menu_items) {
    m_menu_items = menu_items;
    return *this;
}

std::vector<std::string> MenuScreen::getMenuItems() const {
    return m_menu_items;
}

uint8_t MenuScreen::getSelectedMenuItem() const { return m_selected_menu_item; }

MenuScreen& MenuScreen::selectMenuItem(uint8_t index) {
    m_selected_menu_item = index;
    return *this;
}

uint8_t* MenuScreen::build() {
    clear();
    uint16_t y = 0;
    printString(
        0, y,
        std::format("{}/{}", m_selected_menu_item + 1, m_menu_items.size()));
    printString(m_width / 2, y, m_title, {.align = TextAlign::center});

    y += 16;
    auto max_menu_size = m_height / 8 - 1;

    auto start_index = (m_selected_menu_item / max_menu_size) * max_menu_size;
    for (auto i = 0; i < max_menu_size; ++i) {
        if ((start_index + i) >= m_menu_items.size()) {
            break;
        }
        bool is_selected = false;
        if (start_index + i == m_selected_menu_item) {
            is_selected = true;
            drawRectangle(0, y, 1, 8, false);
        }
        printString(1, y, m_menu_items[start_index + i],
                    {.invert = is_selected});
        y += 8;
    }
    return m_frame_buffer;
}