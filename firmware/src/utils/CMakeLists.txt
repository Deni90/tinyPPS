add_library(tinypps_utils INTERFACE)

target_sources(tinypps_utils INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/config.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pdo_helper.cpp
)

target_include_directories(tinypps_utils INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)