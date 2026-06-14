#ifndef screen_h
#define screen_h

#include <cstdint>
#include <span>
#include <string>

class Screen {
  public:
    /**
     * @brief Alias for the frame buffer type
     */
    using FrameBuffer = std::span<uint8_t>;

    /**
     * @brief Constructor
     */
    Screen();

    /**
     * @brief Destructor
     */
    virtual ~Screen() = default;

    /**
     * @brief Initialize the screen with the given frame buffer and dimensions
     *
     * @param[in] frame_buffer The frame buffer to use for the screen
     * @param[in] width The width of the screen
     * @param[in] height The height of the screen
     * @param[in] page_height The height of each page in the frame buffer
     */
    static auto initialize(FrameBuffer frame_buffer, uint16_t width,
                           uint16_t height, uint16_t page_height) -> void;

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
    static inline FrameBuffer m_frame_buffer{};
    static inline uint16_t m_width{0};
    static inline uint16_t m_height{0};
    static inline uint16_t m_page_height{0};
};

#endif   // screen_h
