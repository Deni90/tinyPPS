add_library(tinypps_gui INTERFACE)

target_sources(tinypps_gui INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/menu_screen.cpp
        ${CMAKE_CURRENT_LIST_DIR}/screen.cpp
        ${CMAKE_CURRENT_LIST_DIR}/main_screen.cpp
)

target_include_directories(tinypps_gui INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)