cmake_minimum_required(VERSION 3.13)

add_library(EZ_USB_MIDI_HOST INTERFACE)
target_sources(EZ_USB_MIDI_HOST INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/EZ_USB_MIDI_HOST.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../MIDI_Library/src/MIDI.cpp
)
target_include_directories(EZ_USB_MIDI_HOST INTERFACE
 ${CMAKE_CURRENT_LIST_DIR}
 ${CMAKE_CURRENT_LIST_DIR}/../MIDI_Library/src
)
target_link_libraries(EZ_USB_MIDI_HOST INTERFACE usb_midi_host_app_driver)
