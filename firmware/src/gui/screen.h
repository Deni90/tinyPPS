#ifndef screen_h
#define screen_h

#include <cstdint>
#include <string>

class Screen {
  public:
    /**
     * @brief Constructor
     *
     * @param[in] width Width of the screen
     * @param[in] height Height of the screen
     */
    Screen(uint16_t width, uint16_t height);

    /**
     * @bried Destructor
     */
    virtual ~Screen();

    /**
     * @brief Build function that all screens must to implement
     *
     * This function is used to build the desired screen and store it in the
     * frame buffer.
     *
     * @return Return the buffer containing the screen. Buffer size is always
     * width * height / 8.
     */
    virtual uint8_t* build() = 0;

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
    void clear();

    /**
     * @brief Set, turn on pixel on desired x, y coordinates
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     */
    void setPixel(int16_t x, int16_t y);

    /**
     * @brief A generic function for drawing an image (or whatever buffer) on
     * desired x, y coordianates
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] image Buffer containing the image
     * @param[in] width Width of the image
     * @param[in] height Height of the image
     */
    void draw(int16_t x, int16_t y, const uint8_t* image, uint16_t width,
              uint16_t height, bool invert = false);

    /**
     * @brief A function for drawing a rectange on desired x, y coordinates
     *
     * * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] width Width of the image
     * @param[in] height Height of the image
     * @param[in] fill A flag indicating whether to fill the rectangle. False by default
     */
    void drawRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height,
                       bool fill = false);

    /**
     * @brief Print a single character on desired x, y coordinates
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] ch Charater
     * @param[in] invert Flag indicating printing in inverted mode. It is false
     * by default.
     * @return Return printed character width or 0 in case of error.
     */
    uint16_t printChar(int16_t x, int16_t y, char ch, bool invert = false);

    /**
     * @brief Print a single character double in size
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] ch Charater
     * @param[in] invert Flag indicating printing in inverted mode. It is false
     * by default.
     * @return Return printed character width or 0 in case of error.
     */
    uint16_t printCharBig(int16_t x, int16_t y, char ch, bool invert = false);

    /**
     * @brief Print a null terminated string to x, y coordinates
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] s String
     * @return Return printed text width or 0 in case of error.
     */
    uint16_t printString(int16_t x, int16_t y, const std::string& str);

    /**
     * @brief Print a null terminated string to x, y coordinates
     *
     * @param[in] x X coordianate
     * @param[in] y Y coordinate
     * @param[in] s String
     * @param[in] config Text config
     * @return Return printed text width or 0 in case of error.
     */
    uint16_t printString(int16_t x, int16_t y, const std::string& str,
                         const StringConfig& config);

    uint16_t m_width;
    uint16_t m_height;
    uint8_t* m_frame_buffer;
};

#endif   // screen_h
