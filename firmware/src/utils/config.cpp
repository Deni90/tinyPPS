#include "config.h"

Config::Config()
    : profile(SupplyProfile::unknown), mode(SupplyMode::cv), min_voltage(5000),
      max_voltage(5000), min_current(0), max_current(0), is_menu_enabled(false),
      is_editing_enabled(false) {}

Config ConfigBuilder::buildPdoProfile(int max_voltage, int max_current) {
    Config c;
    c.profile = SupplyProfile::pdo;
    c.mode = SupplyMode::cv;
    c.min_voltage = max_voltage;
    c.max_voltage = max_voltage;
    c.min_current = max_current;
    c.max_current = max_current;
    c.is_editing_enabled = false;
    c.is_menu_enabled = true;
    return c;
}

Config ConfigBuilder::buildPpsProfile(int min_voltage, int max_voltage,
                                      int min_current, int max_current) {
    Config c;
    c.profile = SupplyProfile::pps;
    c.mode = SupplyMode::cv;
    c.min_voltage = min_voltage;
    c.max_voltage = max_voltage;
    c.min_current = min_current;
    c.max_current = max_current;
    c.is_editing_enabled = true;
    c.is_menu_enabled = true;
    return c;
}