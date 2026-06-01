#ifndef screen_h
#define screen_h

#include <array>
#include <cstdint>
#include <string>

constexpr uint16_t k_width = 128;   // Width of the screen in pixels
constexpr uint16_t k_height = 64;   // Height of the screen in pixels
constexpr uint8_t k_page_height = 8;

using FrameBuffer = std::array<uint8_t, k_width * k_height / k_page_height>;

class Screen {
  public:
    /**
     * @brief Constructor
     */
    Screen();

    /**
     * @brief Destructor
     */
    virtual ~Screen() = default;

    /**
     * @brief Build function that all screens must to implement
     *
     * This function is used to build the desired screen and store it in the
     * frame buffer.
     *
     * @return A reference to shared FrameBuffer matching the display
     * dimensions.
     */
    virtual auto build() -> FrameBuffer& = 0;

  protected:
    /**
     * @brief Enumeration describing text alignment
     */
    enum class TextAlign { left, center, right };

    /**
     * @brief Enumeration describing Font sizes
     */
    enum class FontSize { normal, big };

    /**
     * @brief Struct containing string configuration
     */
    struct StringConfig {
        TextAlign align = TextAlign::left;
        FontSize size = FontSize::normal;
        bool invert = false;
    };

    /**
     * Clear the frame buffer
     */
    auto clear() -> void;

    /**
     * @brief Set, turn on pixel on desired x_pos, y_pos coordinates
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     */
    auto setPixel(int16_t x_pos, int16_t y_pos) -> void;

    /**
     * @brief A generic function for drawing an image (or whatever buffer) on
     * desired x_pos, y_pos coordianates
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] object Buffer containing the object/image
     * @param[in] width Width of the image
     * @param[in] height Height of the image
     */
    auto draw(int16_t x_pos, int16_t y_pos, const uint8_t* object,
              uint16_t width, uint16_t height, bool invert = false) -> void;

    /**
     * @brief A function for drawing a rectange on desired x_pos, y_pos
     * coordinates
     *
     * * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] width Width of the image
     * @param[in] height Height of the image
     * @param[in] fill A flag indicating whether to fill the rectangle. False by
     * default
     */
    auto drawRectangle(int16_t x_pos, int16_t y_pos, uint16_t width,
                       uint16_t height, bool fill = false) -> void;

    /**
     * @brief Print a single character on desired x_pos, y_pos coordinates
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] character Charater
     * @param[in] invert Flag indicating printing in inverted mode. It is false
     * by default.
     * @param[in] dry_run If true, do not render the text; only compute the
     * required width.
     * @return Return printed character width or 0 in case of error.
     */
    auto printChar(int16_t x_pos, int16_t y_pos, char character,
                   bool invert = false, bool dry_run = false) -> uint16_t;

    /**
     * @brief Print a single character double in size
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] character Charater
     * @param[in] invert Flag indicating printing in inverted mode. It is false
     * by default.
     * @param[in] dry_run If true, do not render the text; only compute the
     * required width.
     * @return Return printed character width or 0 in case of error.
     */
    auto printCharBig(int16_t x_pos, int16_t y_pos, char character,
                      bool invert = false, bool dry_run = false) -> uint16_t;

    /**
     * @brief Print a null terminated string to x_pos, y_pos coordinates
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] s String
     * @param[in] dry_run If true, do not render the text; only compute the
     * required width.
     * @return Return printed text width or 0 in case of error.
     */
    auto printString(int16_t x_pos, int16_t y_pos, std::string_view str,
                     bool dry_run = false) -> uint16_t;

    /**
     * @brief Print a null terminated string to x_pos, y_pos coordinates
     *
     * @param[in] x_pos X coordianate
     * @param[in] y_pos Y coordinate
     * @param[in] s String
     * @param[in] config Text config
     * @param[in] dry_run If true, do not render the text; only compute the
     * required width.
     * @return Return printed text width or 0 in case of error.
     */
    auto printString(int16_t x_pos, int16_t y_pos, std::string_view str,
                     const StringConfig& config, bool dry_run = false)
        -> uint16_t;

    // Make the frame buffer shared across all screens
    static FrameBuffer m_frame_buffer;
};

#endif   // screen_h
