# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TinyPPS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(TinyPPS src/main.cpp src/config.cpp src/tiny_pps.cpp)

add_subdirectory(src/gui)
add_subdirectory(src/hal)
add_subdirectory(src/ina226)
add_subdirectory(src/rotary_encoder)
add_subdirectory(src/rp_imp)
add_subdirectory(src/ssd1306)

pico_set_program_name(TinyPPS "TinyPPS")
pico_set_program_version(TinyPPS "1.0")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(TinyPPS 0)
pico_enable_stdio_usb(TinyPPS 1)

# Add the standard library to the build
target_link_libraries(TinyPPS
        hardware_i2c
        pico_stdlib
        tinypps_gui
        tinypps_hal
        tinypps_ina226
        tinypps_rotary_encoder
        tinypps_rp_imp
        tinypps_ssd1306
)

pico_add_extra_outputs(TinyPPS)

