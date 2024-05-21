/*
 * @file EZ_USB_MIDI_HOST.h
 * @brief Arduino MIDI Library compatible wrapper for usb_midi_host
 *        application driver
 *
 * This library manages all USB MIDI devices connected to the
 * single USB Root hub for TinyUSB for the whole plug in,
 * operate, unplug lifecycle. It makes each virtual MIDI cable for
 * each connected USB MIDI device behave as if it were a serial port
 * MIDI device and enables applications to use the Arduino MIDI Library
 * (formally called the FortySevenEffects MIDI library) to send and
 * receive MIDI messages between the application and the device.
 *
 * Most applications should only instantiate the EZ_USB_MIDI_HOST
 * class by using the RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE() macro.
 * For example, to instantiate the EZ_USB_MIDI_HOST object with
 * name "usbhMIDI" using the default configuration, add the following
 * to the application (Note the lack of semicolon after the macro):
 *
 * RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, MidiHostSettingsDefault)
 *
 * If you need to customize the settings, make a subclass of the
 * MidiHostSettingsDefault class and pass the name of your new class instead.
 *
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

extern "C" void rppicomidi_ez_usb_midi_host_set_cbs(void (*mount_cb)(uint8_t devAddr, uint8_t nInCables, uint16_t nOutCables, void*),
					 void (*umount_cb)(uint8_t devAddr, void*), void (*rx_cb)(uint8_t devAddr, uint32_t numPackets, void*), void*);

#include "EZ_USB_MIDI_HOST_Config.h"

#include "EZ_USB_MIDI_HOST_Device.h"

#include "EZ_USB_MIDI_HOST_namespace.h"

BEGIN_EZ_USB_MIDI_HOST_NAMESPACE

using ConnectCallback    = void (*)(uint8_t, uint8_t, uint8_t);
using DisconnectCallback = void (*)(uint8_t);

/// @brief This is the class your application should directly
/// instantiate. It tracks when MIDI devices are connected
/// and disconnected from the root. The Application should implement
/// the onConnect() and onDisconnect() methods to track the
/// USB devAddr value of each connected MIDI device.
///
/// This implementation only supports one USB MIDI host port
/// because that is all TinyUSB supports.
template<class settings>
class EZ_USB_MIDI_HOST {
public:
  EZ_USB_MIDI_HOST() : appOnConnect{nullptr}, appOnDisconnect{nullptr} {
        rppicomidi_ez_usb_midi_host_set_cbs(onConnect, onDisconnect, onRx, reinterpret_cast<void*>(this));
        for (uint8_t idx = 0; idx < CFG_TUH_DEVICE_MAX; idx++)
          devAddr2DeviceMap[idx] = nullptr;
    }
  ~EZ_USB_MIDI_HOST() = default;
  EZ_USB_MIDI_HOST(EZ_USB_MIDI_HOST const &) = delete;
  void operator=(EZ_USB_MIDI_HOST const &) = delete;

#ifdef ADAFRUIT_USBH_HOST_H_
  void begin(Adafruit_USBH_Host* usbHost, uint8_t rhPort, ConnectCallback cfptr, DisconnectCallback dfptr) {
    setAppOnConnect(cfptr);
    setAppOnDisconnect(dfptr);
    tuh_midih_define_limits(settings::MidiRxBufsize, settings::MidiTxBufsize, settings::MaxCables);
    usbHost->begin(rhPort);
  }
#else
  void begin(uint8_t rhPort, ConnectCallback cfptr, DisconnectCallback dfptr) {
    setAppOnConnect(cfptr);
    setAppOnDisconnect(dfptr);
    tuh_midih_define_limits(settings::MidiRxBufsize, settings::MidiTxBufsize, settings::MaxCables);
    tuh_init(rhPort);
  }
#endif

  /// @brief test if a MIDI device with the given devAddr is connected
  /// @param devAddr the USB device address of the device to test
  /// @return true if the MIDI device is connected, false otherwise
  bool isConnected(uint8_t devAddr) { return getDevFromDevAddr(devAddr) != nullptr; }

  /// @brief get the number of virtual MIDI IN cables the device supports
  /// @param devAddr the USB device address of the MIDI device
  /// @return the number of MIDI IN virtual cables the connected device supports or
  /// 0 if the device at devAddr is no longer connected
  uint8_t getNumInCables(uint8_t devAddr) { auto ptr = getDevFromDevAddr(devAddr); return ptr != nullptr ? ptr->getNumInCables() : 0; }

  /// @brief get the number of virtual MIDI OUT cables the device supports
  /// @param devAddr the USB device address of the MIDI device
  /// @return the number of MIDI OUT virtual cables the connected device supports
  /// or 0 if the device at devAddr is no longer connected
  uint8_t getNumOutCables(uint8_t devAddr) { auto ptr = getDevFromDevAddr(devAddr); return ptr != nullptr ? ptr->getNumOutCables() : 0; }

  /// @brief Get a pointer to the MidiInterface object associated with a connected MIDI device
  /// @param devAddr the USB device address of a connected MIDI device
  /// @param cable the virtual cable number of the MIDI device the MidiInterface object supports
  /// @return a pointer to the MidiInterface object or nullptr if no object associated
  /// with the devAddr and cable exists (e.g., because the device has been disconnected)
  MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>* getMIDIinterface(uint8_t devAddr, uint8_t cable) {
    auto ptr = getDevFromDevAddr(devAddr);
    return ptr != nullptr ? &(ptr->getMIDIinterface(cable)) : nullptr;
  }

  /// @brief Register a callback function to be called when a MIDI device
  /// is connected
  /// @param ftpr is a pointer to the callback function to be called
  void setAppOnConnect(ConnectCallback fptr) { appOnConnect = fptr; }

  /// @brief Unregister the last callback function that was registered 
  /// to be called when a MIDI device is connected
  void unsetAppOnConnect() { appOnConnect = nullptr; }

  /// @brief Register a callback function to be called when a MIDI device
  /// is connected
  /// @param ftpr is a pointer to the callback function to be called
  void setAppOnDisconnect(DisconnectCallback fptr) { appOnDisconnect = fptr; }

  /// @brief Unregister the last callback function that was registered 
  /// to be called when a MIDI device is disconnected
  void unsetAppOnDisconnect() { appOnConnect = nullptr; }

  /// @brief call the read method for every connected
  /// device's virtual MIDI IN cable. This will trigger the callback
  /// for that device.
  /// @return a bitmap such that bit i is set if cable i has a message ready
  uint16_t readAll() {
    uint16_t hasMessageBitmap = 0;
    for (uint8_t dev = 0; dev < RPPICOMIDI_TUH_MIDI_MAX_DEV; dev++) {
      uint8_t nCables = devices[dev].getNumInCables();
      uint16_t mask = 1;
      for (uint8_t cable = 0; cable < nCables; cable++) {
        if (devices[dev].getMIDIinterface(cable).read()) {
          hasMessageBitmap |= mask;
        }
        mask <<= 1;
      }
    }
    return hasMessageBitmap;
  }

  /// Send as many pending USB MIDI packets as possible to
  /// the connected MIDI devices
  void writeFlushAll() {
    for (uint8_t dev = 0; dev < RPPICOMIDI_TUH_MIDI_MAX_DEV; dev++) {
      devices[dev].writeFlush();
    }
  }

  /// @brief decode hasMessageBitmap returned by the readAll function to check
  /// if a particular virtual MIDI IN cable has a message waiting
  /// @param cable the virtual cable number
  /// @param hasMessageBitmap the value returned by readAll()
  /// @return true if there is a message waiting on the cable, false otherwise
  bool isMessageAvailableOnCable(uint8_t cable, uint16_t hasMessageBitmap) {
    return cable < RPPICOMIDI_TUH_MIDI_MAX_CABLES && (hasMessageBitmap & (1 << cable)) != 0;
  }

  /// @brief Get access to the EZ_USB_MIDI_HOST_Device object associated with the devAddr
  /// @param devAddr the USB device address of the device
  /// @return a pointer to the associated EZ_USB_MIDI_HOST_Device object or nullptr
  /// if there is no device attached to the devAddr
  EZ_USB_MIDI_HOST_Device<settings>* getDevFromDevAddr(uint8_t devAddr) {
    if (devAddr == 0) // 0 is an unconfigured device
        return nullptr;
    uint8_t idx = 0;
    EZ_USB_MIDI_HOST_Device<settings>* ptr = nullptr;
    for (; idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && devAddr2DeviceMap[idx] != nullptr && devAddr2DeviceMap[idx]->getDevAddr() != devAddr; idx++) {}
    if (idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && devAddr2DeviceMap[idx] != nullptr && devAddr2DeviceMap[idx]->getDevAddr() == devAddr) {
      ptr = devAddr2DeviceMap[idx];
    }
    return ptr;
  }

  /// @brief get a pointer to the MIDI Interface object associated with the USB device address
  /// and MIDI virtual IN cable number
  /// @param devAddr the USB device address
  /// @param cable the virtual MIDI IN cable number
  /// @return a pointer to the MIDI Interface object associated with the devAddr and cable
  /// or nullptr if no such interface exists (if, for example, the device was unplugged)
  MIDI_NAMESPACE::MidiInterface<EZ_USB_MIDI_HOST_Transport<settings>, settings>* getInterfaceFromDeviceAndCable(uint8_t devAddr, uint8_t cable) {
    auto dev = getDevFromDevAddr(devAddr);
    if (dev != nullptr && cable < RPPICOMIDI_TUH_MIDI_MAX_CABLES && (cable < dev->getNumInCables() || cable < dev->getNumOutCables()))
      return  &dev->getMIDIinterface(cable);
    return nullptr;
  }

  // The following 3 functions should only be used by the tuh_midi_*_cb()
  // callback functions in file EZ_USB_MIDI_HOST.cpp.
  // They are declared public because the tuh_midi_*cb() callbacks are not
  // associated with any object and this class is accessed via the getInstance()
  // function.
  static void onConnect(uint8_t devAddr, uint8_t nInCables, uint16_t nOutCables, void* inst) {
    auto me = reinterpret_cast<EZ_USB_MIDI_HOST<settings>*>(inst);
    // try to allocate a EZ_USB_MIDI_HOST_Device object for the connected device
    uint8_t idx = 0;
    for (; idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && me->devAddr2DeviceMap[idx] != nullptr; idx++) {}
    if (idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && me->devAddr2DeviceMap[idx] == nullptr) {
      me->devAddr2DeviceMap[idx] = me->devices + idx;
      me->devAddr2DeviceMap[idx]->onConnect(devAddr, nInCables, nOutCables);
      if (me->appOnConnect) me->appOnConnect(devAddr, nInCables, nOutCables);
    }
  }
  static void onDisconnect(uint8_t devAddr, void* inst) {
    auto me = reinterpret_cast<EZ_USB_MIDI_HOST<settings>*>(inst);
    // find the EZ_USB_MIDI_HOST_Device object allocated for this device
    auto ptr = me->getDevFromDevAddr(devAddr);
    if (ptr != nullptr) {
    ptr->onDisconnect(devAddr);
    if (me->appOnDisconnect)
      me->appOnDisconnect(devAddr);
    uint8_t idx = 0;
    for (; idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && me->devAddr2DeviceMap[idx] != nullptr && me->devAddr2DeviceMap[idx]->getDevAddr() != devAddr; idx++) {}
      if (idx < RPPICOMIDI_TUH_MIDI_MAX_DEV && me->devAddr2DeviceMap[idx] != nullptr && me->devAddr2DeviceMap[idx]->getDevAddr() == devAddr) {
        me->devAddr2DeviceMap[idx] = nullptr;
      }
    }
  }
  static void onRx(uint8_t devAddr, uint32_t numPackets, void* inst) {
    auto me = reinterpret_cast<EZ_USB_MIDI_HOST<settings>*>(inst);
    if (numPackets != 0)
    {
      uint8_t cable;
      uint8_t buffer[48];
      while (1) {
        uint16_t bytesRead = tuh_midi_stream_read(devAddr, &cable, buffer, sizeof(buffer));
        if (bytesRead == 0)
          return;
        auto dev = me->getDevFromDevAddr(devAddr);
        if (dev != nullptr) {
          dev->writeToInFIFO(cable, buffer, bytesRead);
        }
      }
    }
  }
private:
  EZ_USB_MIDI_HOST_Device<settings> devices[RPPICOMIDI_TUH_MIDI_MAX_DEV];
  ConnectCallback appOnConnect;
  DisconnectCallback appOnDisconnect;

  // devAddr2DeviceMap[idx] == a pointer to an address if device idx
  // has been connected or nullptr if not.
  // The problem this solves is RPPICOMIDI_TUH_MIDI_MAX_DEV < CFG_TUH_DEVICE_MAX
  EZ_USB_MIDI_HOST_Device<settings>* devAddr2DeviceMap[CFG_TUH_DEVICE_MAX];
};

END_EZ_USB_MIDI_HOST_NAMESPACE

#define RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(name_, settings) \
    static EZ_USB_MIDI_HOST<settings> name_;

