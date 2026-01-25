#include "loading_screen.h"

LoadingScreen::LoadingScreen(uint16_t width, uint16_t height)
    : Screen(width, height), m_progress(0) {}

uint8_t* LoadingScreen::build() {
    clear();
    // TODO add logo here
    // if PDO count is not set show the progress bar, otherwise show the number
    // of PDO profiles
    if (m_pdo_profile_count.has_value()) {
        std::string profiles_found_text =
            std::to_string(m_pdo_profile_count.value()) + " PDOs found";
        printString(m_width / 2, 48, profiles_found_text,
                    {.align = TextAlign::center});
    } else {
        std::string loading_text(m_progress, '.');
        printString(m_width / 2, 50, loading_text,
                    {.align = TextAlign::center, .size = FontSize::big});
        printString(m_width / 2, 48, "Loading PDOs",
                    {.align = TextAlign::center});
    }

    return m_frame_buffer;
}

LoadingScreen& LoadingScreen::updateProgress() {
    m_progress = ++m_progress % 4;
    return *this;
}

LoadingScreen& LoadingScreen::setPdoProfileCount(uint8_t count) {
    m_pdo_profile_count = count;
    return *this;
}