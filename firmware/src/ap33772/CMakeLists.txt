add_library(tinypps_ap33772 INTERFACE)

target_sources(tinypps_ap33772 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ap33772.cpp
)

target_include_directories(tinypps_ap33772 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)