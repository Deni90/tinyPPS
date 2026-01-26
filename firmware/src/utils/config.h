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
    Ap33772s::PdoType pdo_type;
    SupplyMode supply_mode;
    int min_voltage;
    int max_voltage;
    int min_current;
    int max_current;
    bool is_menu_enabled;
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
     * @brief Builds config for PPS profile
     *
     * @param[in] min_voltage Minimum voltage supported by the PPS profile
     * @param[in] max_voltage Maximum voltage supported by the PPS profile
     * @param[in] min_current Minimum current supported by the PDO profile
     * @param[in] max_current Maximum current supported by the PDO profile
     * @return Config object
     */
    static Config buildPpsProfile(int min_voltage, int max_voltage,
                                  int min_current, int max_current);
};
#endif   // config_h