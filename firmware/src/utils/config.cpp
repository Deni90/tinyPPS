#include "config.h"
#include "pdsink_iface.h"

auto ConfigBuilder::buildDefault() -> Config { return Config(); }

auto ConfigBuilder::buildWithPdo(const IPdSink::Pdo& pdo) -> Config {
    Config config;
    config.pdo = pdo;
    config.supply_mode = SupplyMode::CV;
    config.is_editing_enabled = !(pdo.type == IPdSink::PdoType::NONE);
    return config;
}
