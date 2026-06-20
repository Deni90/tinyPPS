#ifndef pdo_helper_hpp
#define pdo_helper_hpp

#include <string_view>

#include "pdsink_iface.hpp"
#include "tiny_format.hpp"

/**
 * @brief Convert a PDO to string
 *
 * @param[in] pdo PDO
 * @param[out] dest Destination buffer
 * @return A string representation od a PDO containing the type, voltage and
 * current range(s)
 */
template <size_t N>
auto pdoToString(const IPdSink::Pdo& pdo, std::array<char, N>& dest)
    -> std::string_view {
    if (pdo.type == IPdSink::PdoType::FIX) {
        return tinyFormat(dest, "FIX %.1fV ^%.1fA", pdo.voltage_max / 1000.0,
                          pdo.current_max / 1000.0);
    }
    if (pdo.type == IPdSink::PdoType::PPS) {
        return tinyFormat(dest, "PPS %.1f-%.1fV ^%.1fA",
                          pdo.voltage_min / 1000.0, pdo.voltage_max / 1000.0,
                          pdo.current_max / 1000.0);
    }
    if (pdo.type == IPdSink::PdoType::AVS) {
        return tinyFormat(dest, "AVS %.1f-%.1fV ^%.1fA",
                          pdo.voltage_min / 1000.0, pdo.voltage_max / 1000.0,
                          pdo.current_max / 1000.0);
    }
    return std::string_view{"N/A"};
}

#endif   // pdo_helper_hpp
