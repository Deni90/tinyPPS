#include "config.h"

Config::Config() : supply_mode(SupplyMode::CV), is_editing_enabled(false) {}

Config ConfigBuilder::buildDefault() { return Config(); }

Config ConfigBuilder::buildWithPdo(const Ap33772s::Pdo& pdo) {
    Config c;
    c.pdo = pdo;
    c.supply_mode = SupplyMode::CV;
    if (pdo.type == Ap33772s::PdoType::NONE) {
        c.is_editing_enabled = false;
    } else {
        c.is_editing_enabled = true;
    }
    return c;
}