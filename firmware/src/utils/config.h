#ifndef config_h
#define config_h

#include "ap33772s.h"

/**
 * @brief Enum representing different supply modes
 */
enum class SupplyMode { CV, CC };

/**
 * @brief Covert SupplyMode enum value to string
 *
 * @param mode SupplyMode
 */
constexpr const char* supplyModeToString(SupplyMode mode) {
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
    Ap33772s::Pdo pdo;
    SupplyMode supply_mode;
    bool is_editing_enabled;
    /**
     * @brief Default constructor
     */
    Config();
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
    static Config buildDefault();

    /**
     * @brief Builds config with PDO
     *
     * @param[in] pdo PDO
     * @return Config object
     */
    static Config buildWithPdo(const Ap33772s::Pdo& pdo);
};
#endif   // config_h