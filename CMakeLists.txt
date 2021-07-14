cmake_minimum_required(VERSION 3.12)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()
PROJECT(usb_sound_card)
file(GLOB sources *.c *.h)
add_executable(usb_sound_card ${sources})
target_link_libraries(usb_sound_card pico_stdlib hardware_pio pico_multicore pico_fix_rp2040_usb_device_enumeration)
pico_add_extra_outputs(usb_sound_card)
