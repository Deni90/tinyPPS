#ifndef tiny_format_hpp
#define tiny_format_hpp

#include <cstdio>
#include <string>

static constexpr size_t kBufferSize = 64;

/**
 * @brief Formats a string using the given format and arguments
 *
 * This function uses `std::snprintf` to format the string.
 *
 * @param format The format string
 * @param args The arguments to format
 * @return The formatted string
 * @note The buffer size is limited to 64 characters
 */
template <typename... Args>
std::string tinyFormat(const char* format, Args... args) {
    char buf[kBufferSize];
    std::snprintf(buf, sizeof(buf), format, args...);
    return std::string(buf);
}

#endif   // tiny_format_hpp
