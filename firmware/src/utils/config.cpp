#include "config.h"

Config::Config()
    : pdo_type(Ap33772s::PdoType::NONE), supply_mode(SupplyMode::CV),
      min_voltage(5000), max_voltage(5000), min_current(1000),
      max_current(1000), is_editing_enabled(false) {}

Config ConfigBuilder::buildDefault() { return Config(); }

Config ConfigBuilder::buildPpsProfile(int min_voltage, int max_voltage,
                                      int min_current, int max_current) {
    Config c;
    c.pdo_type = Ap33772s::PdoType::PPS;
    c.supply_mode = SupplyMode::CV;
    c.min_voltage = min_voltage;
    c.max_voltage = max_voltage;
    c.min_current = min_current;
    c.max_current = max_current;
    c.is_editing_enabled = true;
    return c;
}