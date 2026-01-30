add_library(tinypps_pico_hal INTERFACE)

target_sources(tinypps_pico_hal INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/pico_gpio.cpp
        ${CMAKE_CURRENT_LIST_DIR}/pico_i2c.cpp
)

target_include_directories(tinypps_pico_hal INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)

target_link_libraries(tinypps_pico_hal INTERFACE
        hardware_i2c
)