#ifndef config_h
#define config_h

#include "pdsink_iface.h"

/**
 * @brief Enum representing different supply modes
 */
enum class SupplyMode { CV, CC };

/**
 * @brief Covert SupplyMode enum value to string
 *
 * @param mode SupplyMode
 */
constexpr auto supplyModeToString(SupplyMode mode) -> const char* {
    switch (mode) {
    case SupplyMode::CV:
        return "CV";
    case SupplyMode::CC:
        return "CC";
    }
    return "N/A";
}

/**
 * @brief Struct containing configuration parameters describing a supply profile
 */
struct Config {
    IPdSink::Pdo pdo;
    SupplyMode supply_mode{SupplyMode::CV};
    bool is_editing_enabled{false};
    /**
     * @brief Default constructor
     */
    Config() = default;
};

/**
 * @brief Config builder struct
 *
 * Struct containing builder functions for buiding configurations for
 * different kinds of supply profiles
 */
struct ConfigBuilder {
    /**
     * @brief Builds default configuration with no PDO type
     *
     * @return Config object
     */
    static auto buildDefault() -> Config;

    /**
     * @brief Builds config with PDO
     *
     * @param[in] pdo PDO
     * @return Config object
     */
    static auto buildWithPdo(const IPdSink::Pdo& pdo) -> Config;
};
#endif   // config_h
