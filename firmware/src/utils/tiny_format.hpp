#ifndef tiny_format_hpp
#define tiny_format_hpp

#include <cstdio>
#include <string_view>

/**
 * @brief Formats a string using the given format and arguments
 *
 * This function uses `std::snprintf` to format the string.
 *
 * @param dest The destination buffer
 * @param format The format string
 * @param args The arguments to format
 * @return The formatted string
 * @note The buffer size is limited to N characters
 */
template <size_t N, typename... Args>
auto tinyFormat(std::array<char, N>& dest, const char* format, Args... args)
    -> std::string_view {
    auto written = std::snprintf(dest.data(), dest.size(), format, args...);
    if (written < 0 || static_cast<size_t>(written) >= dest.size()) {
        return std::string_view{};
    }
    return std::string_view(dest.data(), static_cast<size_t>(written));
}

#endif   // tiny_format_hpp
