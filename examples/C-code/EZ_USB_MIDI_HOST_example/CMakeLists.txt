cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)

set(target_proj EZ_USB_MIDI_HOST_example)
project(${target_proj} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../.. EZ_USB_MIDI_HOST)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../../usb_midi_host usb_midi_host)
add_executable(${target_proj}
    EZ_USB_MIDI_HOST_example.cpp
)

pico_enable_stdio_uart(${target_proj} 1)

target_include_directories(${target_proj} PRIVATE
 ${CMAKE_CURRENT_LIST_DIR}
)

target_link_options(${target_proj} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_proj} PRIVATE -Wall -Wextra)
target_link_libraries(${target_proj} tinyusb_host tinyusb_board EZ_USB_MIDI_HOST pico_stdlib)
if(DEFINED PICO_BOARD)
if(${PICO_BOARD} MATCHES "pico_w")
message("board is pico_w")
# add additional compile and link options
target_compile_definitions(${target_proj} PRIVATE RPPICOMIDI_PICO_W=1)
target_link_libraries(${target_proj} pico_cyw43_arch_none)
set(RPPICOMIDI_PICO_W 1)
else()
message("board is pico")
endif()
else()
message("board is not defined")
endif()

pico_add_extra_outputs(${target_proj})

