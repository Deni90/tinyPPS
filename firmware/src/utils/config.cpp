#include "config.h"
#include "pdsink_iface.h"

Config::Config() : supply_mode(SupplyMode::CV), is_editing_enabled(false) {}

Config ConfigBuilder::buildDefault() { return Config(); }

Config ConfigBuilder::buildWithPdo(const IPdSink::Pdo& pdo) {
    Config c;
    c.pdo = pdo;
    c.supply_mode = SupplyMode::CV;
    if (pdo.type == IPdSink::PdoType::NONE) {
        c.is_editing_enabled = false;
    } else {
        c.is_editing_enabled = true;
    }
    return c;
}