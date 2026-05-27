# erro de ld -> lib | provavelmente typo ou algo do tipo

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
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mqtt_temp_led_manip C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(mqtt_temp_led_manip mqtt_temp_led_manip.cpp )

pico_set_program_name(mqtt_temp_led_manip "mqtt_temp_led_manip")
pico_set_program_version(mqtt_temp_led_manip "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(mqtt_temp_led_manip 0)
pico_enable_stdio_usb(mqtt_temp_led_manip 1)

# Add the standard library to the build
target_link_libraries(mqtt_temp_led_manip
        pico_stdlib
        hardware_adc
        pico_lwip_mqtt
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        tinyusb_board
        )

# Add the standard include files to the build
target_include_directories(mqtt_temp_led_manip PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(mqtt_temp_led_manip 
        pico_cyw43_arch_none
        )

pico_add_extra_outputs(mqtt_temp_led_manip)

