#ifndef pdo_helper_h
#define pdo_helper_h

#include <string>

#include "pdsink_iface.h"

/**
 * @brief Convert a PDO to string
 *
 * @param[in] pdo PDO
 * @return A string representation od a PDO containing the type, voltage and
 * current range(s)
 */
std::string pdoToString(const IPdSink::Pdo& pdo);

#endif   // pdo_helper_h