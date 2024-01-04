cmake_minimum_required(VERSION 3.13)

add_library(rppicomidi_USBH_MIDI INTERFACE)
target_sources(rppicomidi_USBH_MIDI INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/rppicomidi_USBH_MIDI.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../MIDI_Library/src/MIDI.cpp
)
target_include_directories(rppicomidi_USBH_MIDI INTERFACE
 ${CMAKE_CURRENT_LIST_DIR}
 ${CMAKE_CURRENT_LIST_DIR}/../MIDI_Library/src
)
target_link_libraries(rppicomidi_USBH_MIDI INTERFACE usb_midi_host_app_driver)
