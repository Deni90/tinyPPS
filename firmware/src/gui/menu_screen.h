#ifndef menu_screen_h
#define menu_screen_h

#include "screen.h"

#include <string>
#include <vector>

class MenuScreen : public Screen {
  public:
    /**
     * @brief Constructor
     */
    MenuScreen() = default;

    /**
     * @brief Destructor
     */
    ~MenuScreen() override = default;

    /**
     * @brief Get the menu title
     *
     * @return Menu title
     */
    [[nodiscard]] auto getTitle() const -> std::string;

    /**
     * @brief Set menu title
     *
     * @param[in] title Title
     * @return reference to this menu screen object
     */
    auto setTitle(const std::string& title) -> MenuScreen&;

    /**
     * @brief Get menu items
     *
     * @return Vector containing menu items
     */
    [[nodiscard]] auto getMenuItems() const -> std::vector<std::string>;

    /**
     * @brief Populate menu with menu items
     *
     * @param[in] Vector containing menu items
     * @return reference to this menu screen object
     */
    auto setMenuItems(const std::vector<std::string>& menu_items)
        -> MenuScreen&;

    /**
     * @brief Get the index of the selected menu item
     *
     * @return Index of the selected menu item
     */
    auto getSelectedMenuItem() const -> uint8_t;

    /**
     * @brief Select a menu item
     *
     * @param[in] index Index of the selected menu item
     * @return reference to this menu screen object
     */
    auto selectMenuItem(uint8_t index) -> MenuScreen&;

    /**
     * @brief Select the next menu item
     *
     * Wraps around to the first item if the current item is the last one.
     *
     * @return reference to this menu screen object
     */
    auto selectNextMenuItem() -> MenuScreen&;

    /**
     * @brief Select the previous menu item
     *
     * Wraps around to the last item if the current item is the first one.
     *
     * @return reference to this menu screen object
     */
    auto selectPreviousMenuItem() -> MenuScreen&;

    /**
     * @brief Build the menu screen based on data provided by user
     *
     * @return A reference to shared FrameBuffer matching the display
     * dimensions.
     */
    auto build() -> FrameBuffer& override;

  private:
    std::string m_title;
    std::vector<std::string> m_menu_items;
    uint8_t m_selected_menu_item{0};
};

#endif   // menu_screen_h
