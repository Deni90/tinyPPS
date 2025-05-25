#ifndef config_h
#define config_h

/**
 * @brief Enum representing different supply profiles supported
 */
enum class SupplyProfile { unknown, pdo, pps };

/**
 * @brief Enum representing different supply modes
 */
enum class SupplyMode { cv, cc };

/**
 * @brief Struct containing configuration parameters describing a supply profile
 */
struct Config {
    SupplyProfile profile;
    SupplyMode mode;
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
     * @brief Builds config for PDO profile
     *
     * @param[in] max_voltage Maximum voltage supported by the PDO profile
     * @param[in] max_current Maximum current supported by the PDO profile
     * @return Config object
     */
    static Config buildPdoProfile(int max_voltage, int max_current);

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