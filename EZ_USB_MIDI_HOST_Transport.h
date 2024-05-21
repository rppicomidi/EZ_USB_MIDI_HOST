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

#if ARDUINO
#include "Adafruit_TinyUSB.h"
#else
#include "tusb.h"
#endif
#include "midi_Namespace.h"
#include "EZ_USB_MIDI_HOST_namespace.h"

#include "usb_midi_host.h"

BEGIN_EZ_USB_MIDI_HOST_NAMESPACE
/// @brief This class models a MIDI IN and MIDI OUT virtual
/// cable pair of a connected USB MIDI device. It implements
/// the required Transport class of the MIDI interface class.
/// Applications normally do not instantiate this class
/// Use the API for the EZ_USB_MIDI_HOST class instead.
template<class settings>
class EZ_USB_MIDI_HOST_Transport {
public:
  EZ_USB_MIDI_HOST_Transport()  :
    devAddr(0), //not connected
    cableNum(no_cable), // cable number not assigned
    hasMIDI_IN(false), // so doesn't have MIDI IN
    hasMIDI_OUT(false), // or MIDI out
    inFIFOunderflow(false),
    inFIFOoverflow(false),
    outFIFOoverflow(false) {
      // The FIFO is not overwritable
      tu_fifo_config(&inFIFO, &inBuffer, settings::MidiRxBufsize, sizeof(uint8_t), false);
      tu_fifo_clear(&inFIFO);
    }

  /// Return the device address of the connected device,
  /// or 0 if no device is connected
  uint8_t getDevAddr() { return devAddr; }
  
  /// Return true if this stream supports MIDI IN
  bool hasInCable() { return hasMIDI_IN; }

  /// Return true if this stream supports MIDI OUT
  bool hasOutCable() { return hasMIDI_OUT; }

  /// configure stream for MIDI communication
  void setConfiguration(uint8_t devAddr_, uint8_t cableNum_, bool hasMIDI_IN_, bool hasMIDI_OUT_) {
    devAddr = devAddr_;
    cableNum = cableNum_;
    hasMIDI_IN = hasMIDI_IN_;
    hasMIDI_OUT = hasMIDI_OUT_;
    tu_fifo_clear(&inFIFO);
  }

  // Required for MIDI transport interface

  void begin() { tu_fifo_clear(&inFIFO); }

  void end() { setConfiguration(0, no_cable, false, false); }

  /// Return the number of bytes available to read from the inFIFO.
  /// Always return 0 if the transport has no MIDI IN
  uint16_t available() { return hasMIDI_IN ? tu_fifo_count(&inFIFO) : 0; }

  /// return the byte read from the inFIFO. Will be 0 (bogus) if no data is available
  uint8_t read() {
    uint8_t buffer = 0;
    if (hasMIDI_IN) {
      inFIFOunderflow = !tu_fifo_read(&inFIFO, &buffer);
      if (!inFIFOunderflow) {
        inFIFOoverflow = false;
      }
    }
    return buffer;
  }

  /// return true if the MIDI IN buffer was full and the data received
  /// callback attempted to write at least one more byte.
  /// Call read() to clear this error
  bool inOverflow() { return inFIFOoverflow; }

  /// Return true if the last time read was called, the MIDI IN FIFO
  /// was empty.
  bool inUnderflow() { return inFIFOunderflow; }

  /// write the byte to the MIDI stream. No error is reported if something goes wrong
  void write(uint8_t byteToWrite) { outFIFOoverflow = (1 != tuh_midi_stream_write(devAddr, cableNum, &byteToWrite, 1)); }

  /// return true if the last call to write() caused the MIDI OUT FIFO to overflow
  /// Applications should wait for the for this function to return
  /// false before writing more data
  bool outOverflow() { return outFIFOoverflow; }

  /// Signal start of transmission to the transport; return false if
  /// if there is no MIDI OUT in the transport, if there is no connected device,
  /// or if the OUT FIFO is full so subsequent calls to write() will fail
  bool beginTransmission(uint8_t) { return devAddr != 0 && hasMIDI_OUT && tuh_midi_can_write_stream(devAddr); }

  /// signal end of transmission to the transport; nothing to do
  void endTransmission() {  }

  /// The following method is used internally. Applications should not use it
  bool writeToInFIFO(uint8_t* bytes, uint16_t nBytes) {
    uint16_t nWritten = tu_fifo_write_n(&inFIFO, bytes, nBytes);
    if (nWritten < nBytes) {
      inFIFOoverflow = true;
      return false;
    }
    return true;
  }

  static const bool thruActivated = false;

private:
  static uint8_t const no_cable = 16;  //!< legal MIDI cable numbers are 0-15
  uint8_t devAddr;
  uint8_t cableNum;
  bool hasMIDI_IN;
  bool hasMIDI_OUT;

  uint8_t inBuffer[settings::MidiRxBufsize];
  tu_fifo_t inFIFO;
  bool inFIFOunderflow;
  bool inFIFOoverflow;
  bool outFIFOoverflow;
};

END_EZ_USB_MIDI_HOST_NAMESPACE
