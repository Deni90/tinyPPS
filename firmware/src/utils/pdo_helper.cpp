#include "pdo_helper.h"
#include "pdsink_iface.h"

#include <format>

std::string pdoToString(const IPdSink::Pdo& pdo) {
    if (pdo.type == IPdSink::PdoType::FIX) {
        return std::format("FIX {}V ^{}A", pdo.voltage_max / 1000.0,
                           pdo.current_max / 1000.0);
    } else if (pdo.type == IPdSink::PdoType::PPS) {
        return std::format("PPS {}-{}V ^{}A", pdo.voltage_min / 1000.0,
                           pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    } else if (pdo.type == IPdSink::PdoType::AVS) {
        return std::format("AVS {}-{}V ^{}A", pdo.voltage_min / 1000.0,
                           pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    }
    return std::string();
}