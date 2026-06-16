# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
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
set(PICO_BOARD none CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TinyPPS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(TinyPPS
        src/main.cpp
        src/state_machine.cpp
)

add_subdirectory(src/ap33772)
add_subdirectory(src/ap33772s)
add_subdirectory(src/gui)
add_subdirectory(src/hal)
add_subdirectory(src/ina226)
add_subdirectory(src/pico_hal)
add_subdirectory(src/rotary_encoder)
add_subdirectory(src/ssd1306)
add_subdirectory(src/utils)

pico_set_program_name(TinyPPS "TinyPPS")
pico_set_program_version(TinyPPS "2.0")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(TinyPPS 0)
pico_enable_stdio_usb(TinyPPS 1)

# Add the standard library to the build
target_link_libraries(TinyPPS
        hardware_i2c
        pico_stdlib
        tinypps_ap33772
        tinypps_ap33772s
        tinypps_gui
        tinypps_hal
        tinypps_ina226
        tinypps_pico_hal
        tinypps_rotary_encoder
        tinypps_ssd1306
        tinypps_utils
)

target_include_directories(TinyPPS PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

pico_add_extra_outputs(TinyPPS)

#disable RTTI and exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wpedantic -Wextra")

# Custom target to flash the RP2040 via OpenOCD
add_custom_target(flash
    COMMAND openocd -f interface/cmsis-dap.cfg
                    -f target/rp2040.cfg
                    -c "adapter speed 5000"
                    -c "program ${CMAKE_CURRENT_BINARY_DIR}/TinyPPS.elf verify reset exit"
    DEPENDS TinyPPS
    COMMENT "Flashing TinyPPS to RP2040 via OpenOCD..."
    USES_TERMINAL
    VERBATIM
)
