#include "pdo_helper.hpp"
#include "pdsink_iface.hpp"

#include "tiny_format.hpp"

auto pdoToString(const IPdSink::Pdo& pdo) -> std::string {
    if (pdo.type == IPdSink::PdoType::FIX) {
        return tinyFormat("FIX %.1fV ^%.1fA", pdo.voltage_max / 1000.0,
                          pdo.current_max / 1000.0);
    }
    if (pdo.type == IPdSink::PdoType::PPS) {
        return tinyFormat("PPS %.1f-%.1fV ^%.1fA", pdo.voltage_min / 1000.0,
                          pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    }
    if (pdo.type == IPdSink::PdoType::AVS) {
        return tinyFormat("AVS %.1f-%.1fV ^%.1fA", pdo.voltage_min / 1000.0,
                          pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    }
    return {};
}
