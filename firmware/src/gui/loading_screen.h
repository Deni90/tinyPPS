#ifndef loading_screen_h
#define loading_screen_h

#include "screen.h"

#include <optional>

class LoadingScreen : public Screen {
  public:
    /**
     * @brief Constructor
     */
    LoadingScreen() = default;

    /**
     * @brief Destructor
     */
    ~LoadingScreen() override = default;

    /**
     * @brief Build the loading screen based on data provided by user
     *
     * @return A reference to shared FrameBuffer matching the display
     * dimensions.
     */
    auto build() -> FrameBuffer& override;

    /**
     * @brief Update the progress bar
     *
     * The progress bar is represented by max three dots. Every time the
     * function is called the prorgess will be updated with dots, from zero ("")
     * to three ("...").
     */
    auto updateProgress() -> LoadingScreen&;

    /**
     * @brief Set the PDO profile count once the negotiation is completed
     *
     * @param count Number of PDO profiles
     */
    auto setPdoProfileCount(uint8_t count) -> LoadingScreen&;

  private:
    uint8_t m_progress{0};
    std::optional<uint8_t> m_pdo_profile_count;
};

#endif   // loading_screen_h
