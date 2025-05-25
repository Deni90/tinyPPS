add_library(tinypps_rp_imp INTERFACE)

target_sources(tinypps_rp_imp INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/rp_i2c.cpp
)

target_include_directories(tinypps_rp_imp INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)

target_link_libraries(tinypps_rp_imp INTERFACE
        hardware_i2c
)