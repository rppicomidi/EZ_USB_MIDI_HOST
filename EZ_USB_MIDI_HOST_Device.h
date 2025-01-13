/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once
#include "MIDI.h"
#include "EZ_USB_MIDI_HOST_Transport.h"

#include "EZ_USB_MIDI_HOST_namespace.h"

BEGIN_EZ_USB_MIDI_HOST_NAMESPACE


/// @brief This class models a connected USB MIDI device
/// Applications normally do not instantiate this class
/// Use the API for the EZ_USB_MIDI_HOST class instead.
template<class settings>
class EZ_USB_MIDI_HOST_Device {
public:
  EZ_USB_MIDI_HOST_Device() : devAddr{0}, nInCables{0}, nOutCables{0}, vid{0}, pid{0}, onMidiInWriteFail{nullptr} {
    clearTransports();
    for (unsigned idx=0;idx < settings::MaxCables; idx++) {
        interfaces[idx] = new MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>(transports[idx]);
    }
  }

  ~EZ_USB_MIDI_HOST_Device() {
    for (unsigned idx=0;idx < settings::MaxCables; idx++) {
        delete interfaces[idx];
    }
  }

  /// Call this function to configure the MIDI interface objects
  /// associated with the device's virtual MIDI cables
  void onConnect(uint8_t devAddr_, uint8_t nInCables_, uint8_t nOutCables_) {
    if (devAddr_ > 0 && devAddr_ <= RPPICOMIDI_TUH_MIDI_MAX_DEV) {
        devAddr = devAddr_;
        nInCables = nInCables_;
        nOutCables = nOutCables_;
        clearTransports(); // make sure all transports are initialized
        uint8_t maxCables = nInCables > nOutCables ? nInCables : nOutCables;
        for (uint8_t idx = 0; idx < maxCables; idx++) {
            transports[idx].setConfiguration(devAddr, idx, idx < nInCables, idx < nOutCables);
            interfaces[idx]->begin(MIDI_CHANNEL_OMNI);
        }
        tuh_vid_pid_get(devAddr, &vid, &pid);
    }
  }

  /// @brief Call this function to unconfigure all MIDI interface objects
  /// associated with the device's virtual MIDI cables
  /// @param devAddr_ is currently not used
  void onDisconnect(uint8_t devAddr_) {
    (void)devAddr_;
    clearTransports();
  }

  /// @brief  
  /// @return the device address for this device object
  uint8_t getDevAddr() { return devAddr; }

  /// @brief 
  /// @return the number of virtual MIDI IN cables for this device object 
  uint8_t getNumInCables() { return nInCables; }

  /// @brief 
  /// @return the number of virtual MIDI OUT cables for this device object 
  uint8_t getNumOutCables() { return nOutCables; }

  /// @brief Get the MIDI interface object associated with a particular virtual MIDI cable
  /// @param cable the virtual MIDI cable
  /// @return a reference to the MIDI interface object
  MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>& getMIDIinterface(uint8_t cable) {
    return *interfaces[cable];
  }

  /// @brief Enqueue message bytes to MIDI IN FIFO of a particular transport
  /// @param cable the virtual cable number of the MIDI interface on the device
  /// @param buffer points to an array of bytes to send
  /// @param nBytes the number of bytes in the buffer to send
  void writeToInFIFO(uint8_t cable, uint8_t* buffer, uint16_t nBytes) {
    if (cable < nInCables) {
      if (!transports[cable].writeToInFIFO(buffer, nBytes)) {
        if (onMidiInWriteFail != nullptr) {
          onMidiInWriteFail(devAddr, cable, transports[cable].inOverflow());
        }
      }
    }
  }

  /// @brief register a callback function that is called if the USB receive
  /// callback fails to write the received data to the FIFO
  /// @param fptr a pointer to the callback function; 
  void setOnMidiInWriteFail(void (*fptr)(uint8_t devAddr, uint8_t cable, bool fifoOverflow)) {onMidiInWriteFail = fptr; }

  /// @brief Send any queued bytes to the connected device
  /// if the host bus is ready to do it. Does nothing if
  /// there is nothing to send or if the host bus is busy
  void writeFlush() {
    if (devAddr != 0)
        tuh_midi_stream_flush(devAddr);
  }

  /// @brief
  ///
  /// @return get Vendor ID for the connected device
  uint16_t get_vid() { return vid; }

  /// @brief
  ///
  /// @return get Product ID for the connected device
  uint16_t get_pid() { return pid; }

private:
  void clearTransports() {
    for (uint8_t idx = 0; idx < settings::MaxCables; idx++) {
        transports[idx].end();
    }
  }
  uint8_t devAddr;
  uint8_t nInCables;
  uint8_t nOutCables;
  uint16_t vid;
  uint16_t pid;
  void (*onMidiInWriteFail)(uint8_t devAddr, uint8_t cable, bool fifoOverflow);
  EZ_USB_MIDI_HOST_Transport<settings> transports[settings::MaxCables];
  MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>* interfaces[settings::MaxCables];
};

END_EZ_USB_MIDI_HOST_NAMESPACE
