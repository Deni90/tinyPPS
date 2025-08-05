add_library(tinypps_ina226 INTERFACE)

target_sources(tinypps_ina226 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ina226.cpp
)

target_include_directories(tinypps_ina226 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)