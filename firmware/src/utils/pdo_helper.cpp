#include "pdo_helper.h"

#include <format>

std::string pdoToString(const Ap33772s::Pdo& pdo) {
    if (pdo.type == Ap33772s::PdoType::FIX) {
        return std::format("FIX {}V ^{}A", pdo.voltage_max / 1000.0,
                           pdo.current_max / 1000.0);
    } else if (pdo.type == Ap33772s::PdoType::PPS) {
        return std::format("PPS {}-{}V ^{}A", pdo.voltage_min / 1000.0,
                           pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    } else if (pdo.type == Ap33772s::PdoType::AVS) {
        return std::format("AVS {}-{}V ^{}A", pdo.voltage_min / 1000.0,
                           pdo.voltage_max / 1000.0, pdo.current_max / 1000.0);
    }
    return std::string();
}