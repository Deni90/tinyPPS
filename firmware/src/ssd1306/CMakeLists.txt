add_library(tinypps_ssd1306 INTERFACE)

target_sources(tinypps_ssd1306 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ssd1306.cpp
)

target_include_directories(tinypps_ssd1306 INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)