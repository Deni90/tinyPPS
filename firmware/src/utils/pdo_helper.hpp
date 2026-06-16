#ifndef pdo_helper_hpp
#define pdo_helper_hpp

#include <string>

#include "pdsink_iface.hpp"

/**
 * @brief Convert a PDO to string
 *
 * @param[in] pdo PDO
 * @return A string representation od a PDO containing the type, voltage and
 * current range(s)
 */
auto pdoToString(const IPdSink::Pdo& pdo) -> std::string;

#endif   // pdo_helper_hpp
