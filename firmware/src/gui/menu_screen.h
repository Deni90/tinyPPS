#ifndef menu_screen_h
#define menu_screen_h

#include "screen.h"

#include <string>
#include <vector>

class MenuScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screeen
     */
    MenuScreen(uint16_t width, uint16_t height);

    /**
     * @brief Destructor
     */
    ~MenuScreen() = default;

    /**
     * @brief Get the menu title
     *
     * @return Menu title
     */
    std::string getTitle() const;

    /**
     * @brief Set menu title
     *
     * @param[in] title Title
     * @return reference to this menu screen object
     */
    MenuScreen& setTitle(const std::string& title);

    /**
     * @brief Get menu items
     *
     * @return Vector containing menu items
     */
    std::vector<std::string> getMenuItems() const;

        /**
         * @brief Populate menu with menu items
         *
         * @param[in] Vector containing menu items
         * @return reference to this menu screen object
         */
        MenuScreen& setMenuItems(const std::vector<std::string>& menu_items);

    /**
     * @brief Get the index of the selected menu item
     *
     * @return Index of the selected menu item
     */
    uint8_t getSelectedMenuItem() const;

    /**
     * @brief Select a menu item
     *
     * @param[in] index Index of the selected menu item
     * @return reference to this menu screen object
     */
    MenuScreen& selectMenuItem(uint8_t index);

    /**
     * @brief Build the menu screen based on data provided by user
     *
     * @return An array containing the menu screen matching the screen
     * dimensions
     */
    virtual uint8_t* build() override;

  private:
    std::string m_title;
    std::vector<std::string> m_menu_items;
    uint8_t m_selected_menu_item;
};

#endif   // menu_screen_h