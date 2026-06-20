#include "config.hpp"
#include "pdo_helper.hpp"
#include "pdsink_iface.hpp"

auto ConfigBuilder::buildDefault() -> Config { return Config(); }

auto ConfigBuilder::buildWithPdo(const IPdSink::Pdo& pdo) -> Config {
    Config config;
    config.pdo = pdo;
    config.is_editing_enabled = !(pdo.type == IPdSink::PdoType::NONE);
    return config;
}
