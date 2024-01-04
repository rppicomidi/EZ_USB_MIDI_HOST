/**
 * @file EZ_USB_MIDI_HOST.cpp
 * @brief Arduino MIDI Library compatible wrapper for usb_midi_host
 *        application driver
 *
 * See the file EZ_USB_MIDI_HOST.h for usage documentation
 *
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "EZ_USB_MIDI_HOST.h"

BEGIN_EZ_USB_MIDI_HOST_NAMESPACE
EZ_USB_MIDI_HOST* EZ_USB_MIDI_HOST::instance=nullptr;
END_EZ_USB_MIDI_HOST_NAMESPACE

/* TinyUSB callbacks */
void tuh_midi_mount_cb(uint8_t devAddr, uint8_t inEP, uint8_t outEP, uint8_t nInCables, uint16_t nOutCables)
{
  (void)inEP;
  (void)outEP;
  EZ_USB_MIDI_HOST_NAMESPACE::EZ_USB_MIDI_HOST::getInstance()->onConnect(devAddr, nInCables, nOutCables);
}

void tuh_midi_umount_cb(uint8_t devAddr, uint8_t unused)
{
  (void)unused;
  EZ_USB_MIDI_HOST_NAMESPACE::EZ_USB_MIDI_HOST::getInstance()->onDisconnect(devAddr);
}

void tuh_midi_rx_cb(uint8_t devAddr, uint32_t numPackets)
{
  if (numPackets != 0)
  {
    uint8_t cable;
    uint8_t buffer[48];
    while (1) {
      uint16_t bytesRead = tuh_midi_stream_read(devAddr, &cable, buffer, sizeof(buffer));
      if (bytesRead == 0)
        return;
      auto dev = EZ_USB_MIDI_HOST_NAMESPACE::EZ_USB_MIDI_HOST::getInstance()->getDevFromDevAddr(devAddr);
      if (dev != nullptr) {
        dev->writeToInFIFO(cable, buffer, bytesRead);
      }
    }
  }
}
