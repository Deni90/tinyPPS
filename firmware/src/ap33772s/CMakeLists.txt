add_library(tinypps_ap33772s INTERFACE)

target_sources(tinypps_ap33772s INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ap33772s.cpp
)

target_include_directories(tinypps_ap33772s INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)