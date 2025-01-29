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

  /// convert the UTF-16le string from a USB string descriptor to a UTF-8 C-string
  /// Only works with single 16-bit word src characters; converts 2-word characters
  /// to a single byte space character
  /// See https://en.wikipedia.org/wiki/UTF-8 and https://en.wikipedia.org/wiki/UTF-16
  void utf16le2utf8(uint16_t* src, size_t maxsrc, uint8_t* dest, size_t maxdest) {
    size_t destidx = 0;
    size_t srcidx = 1; // The first word contains the string length in word and the descriptor type
    size_t srclen = ((src[0] & 0xff) - 2) / 2;
    if (srclen < maxsrc)
        maxsrc = srclen + 1;
    for(;;) {
      // assume a 0 word in the src array is a null termination
      if (srcidx >= maxsrc || src[srcidx] == 0 || (destidx+1) >= maxdest) {
        dest[destidx] = 0;
        break;
      }
      else if (src[srcidx] < 0x80) {
        dest[destidx++] = src[srcidx++] & 0x7f;
      }
      else if (src[srcidx] < 0x800) {
        if ((destidx+2) >= maxdest) {
          // no room for a 2-byte code
          dest[destidx] = 0;
          break;
        }
        else {
          dest[destidx++] = 0xC0 | ((src[srcidx] >> 6) & 0x1f);
          dest[destidx++] = 0x80 | (src[srcidx++] & 0x3f);
        }
      }
      else if (src[srcidx] < 0xD800 || src[srcidx] >= 0xE000) {
        if ((destidx+3) >= maxdest) {
          // no room for a 3-byte code
          dest[destidx] = 0;
          break;
        }
        else {
          dest[destidx++] = 0xE0 | ((src[srcidx] >> 12) & 0xf);
          dest[destidx++] = 0x80 | ((src[srcidx] >> 6) & 0x3f);
          dest[destidx++] = 0x80 | (src[srcidx++] & 0x3f);
        }
      }
      else {
        // paired surrogate; output is space and skipping single encoding
        dest[destidx++] = ' ';
        srcidx +=2;
      }
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
        const uint16_t languageID = 0x0409;
        uint16_t buf[256];

        memset(buf, 0, sizeof(buf));
        uint8_t xfer_result = tuh_descriptor_get_manufacturer_string_sync(devAddr, languageID, buf, sizeof(buf));
        if (XFER_RESULT_SUCCESS == xfer_result) {
          utf16le2utf8(buf, 256, manufacturerStr, maxDevStr);
        }

        memset(buf, 0, sizeof(buf));
        xfer_result = tuh_descriptor_get_product_string_sync(devAddr, languageID, buf, sizeof(buf));
        if (XFER_RESULT_SUCCESS == xfer_result) {
          utf16le2utf8(buf, 256, productStr, maxDevStr);
        }
        memset(buf, 0, sizeof(buf));
        xfer_result = tuh_descriptor_get_serial_string_sync(devAddr, languageID, buf, sizeof(buf));
        if (XFER_RESULT_SUCCESS == xfer_result) {
          utf16le2utf8(buf, 256, serialStr, maxDevStr);
        }
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
  uint16_t getVID() { return vid; }

  /// @brief
  ///
  /// @return get Product ID for the connected device
  uint16_t getPID() { return pid; }

  /// @brief
  /// @return the null-terminated Product string if the device has one
  const uint8_t* getProductStr() {return productStr; }

  /// @brief
  /// @return the null-terminated Manufacturer string if the device has one
  const uint8_t* getManufacturerStr() {return manufacturerStr; }

  /// @brief
  /// @return the null-terminated Serial string if the device has one
  const uint8_t* getSerialString() {return serialStr; }
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
  static const size_t maxDevStr = 512;
  uint8_t productStr[maxDevStr];
  uint8_t manufacturerStr[maxDevStr];
  uint8_t serialStr[maxDevStr];
  void (*onMidiInWriteFail)(uint8_t devAddr, uint8_t cable, bool fifoOverflow);
  EZ_USB_MIDI_HOST_Transport<settings> transports[settings::MaxCables];
  MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>* interfaces[settings::MaxCables];
};

END_EZ_USB_MIDI_HOST_NAMESPACE
