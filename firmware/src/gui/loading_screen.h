#ifndef loading_screen_h
#define loading_screen_h

#include "screen.h"

#include <optional>

class LoadingScreen : public Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screeen
     */
    LoadingScreen(uint16_t width, uint16_t height);

    /**
     * @brief Destructor
     */
    ~LoadingScreen() = default;

    /**
     * @brief Build the loading screen based on data provided by user
     *
     * @return An array containing the loading screen matching the screen
     * dimensions
     */
    virtual uint8_t* build() override;

    /**
     * @brief Update the progress bar
     *
     * The progress bar is represented by max three dots. Every time the
     * function is called the prorgess will be updated with dots, from zero ("")
     * to three ("...").
     */
    LoadingScreen& updateProgress();

    /**
     * @brief Set the PDO profile count once the negotiation is completed
     *
     * @param count Number of PDO profiles
     */
    LoadingScreen& setPdoProfileCount(uint8_t count);

  private:
    uint8_t m_progress;
    std::optional<uint8_t> m_pdo_profile_count;
};

#endif   // loading_screen_h