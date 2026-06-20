#ifndef menu_screen_hpp
#define menu_screen_hpp

#include "config.hpp"
#include "screen.hpp"

#include <span>
#include <string_view>

class MenuScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] title Menu title
     */
    MenuScreen(std::string_view title);

    /**
     * @brief Destructor
     */
    ~MenuScreen() override = default;

    /**
     * @brief Get the menu title
     *
     * @return Menu title
     */
    [[nodiscard]] auto getTitle() const -> std::string_view;

    /**
     * @brief Set config
     *
     * @param[in] config Span containing PDOs that can be used as menu items
     * @return reference to this menu screen object
     */
    auto setConfig(std::span<const Config> config) -> MenuScreen&;

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
    std::string_view m_title;
    std::span<const Config> m_config;
    uint8_t m_selected_menu_item{0};
};

#endif   // menu_screen_hpp
