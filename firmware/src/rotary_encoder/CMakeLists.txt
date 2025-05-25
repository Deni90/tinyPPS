add_library(tinypps_rotary_encoder INTERFACE)

target_sources(tinypps_rotary_encoder INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/rotary_encoder.cpp
)

target_include_directories(tinypps_rotary_encoder INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)