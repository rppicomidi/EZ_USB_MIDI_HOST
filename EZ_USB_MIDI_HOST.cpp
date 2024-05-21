/**
 *
 * The MIT License (MIT)
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

/**
 * @file EZ_USB_MIDI_HOST.cpp
 * This file contains the C callback functions for the usb_midi_host library.
 * These callbacks have to be defined here because the Arduino IDE will not
 * link these functions to the usb_midi_host library if they are defined in
 * the sketch directory. This adds a single function call overhead to the
 * callbacks over using the raw usb_midi_host library.
 */
#include <cstdint>

static void* inst_ptr = nullptr; //!< a pointer to the instance of the EZ_USB_MIDI_HOST class that the application created
/* The following are pointers to the callback functions implemented in the EZ_USB_MIDI_HOST class */
static void (*mount_cb_fp)(uint8_t devAddr, uint8_t nInCables, uint16_t nOutCables, void* inst)=nullptr;
static void (*umount_cb_fp)(uint8_t devAddr, void* inst)=nullptr;
static void (*rx_cb_fp)(uint8_t devAddr, uint32_t numPackets, void* inst)=nullptr;

/**
 * @brief Initialize the pointers to the callback functions. The EZ_USB_MIDI_HOST
 * class constructor should call this. Applications probably should not.
 */
extern "C" void rppicomidi_ez_usb_midi_host_set_cbs(void (*mount_cb)(uint8_t devAddr, uint8_t nInCables, uint16_t nOutCables, void*),
					 void (*umount_cb)(uint8_t devAddr, void*), void (*rx_cb)(uint8_t devAddr, uint32_t numPackets, void*),
					 void* inst)
{
  mount_cb_fp = mount_cb;
  umount_cb_fp = umount_cb;
  rx_cb_fp = rx_cb;
  inst_ptr = inst;
}

/* The following functions override the weak functions declared in the usb_midi_host library */

extern "C" void tuh_midi_mount_cb(uint8_t devAddr, uint8_t inEP, uint8_t outEP, uint8_t nInCables, uint16_t nOutCables)
{
  (void)inEP;
  (void)outEP;
  mount_cb_fp(devAddr, nInCables, nOutCables, inst_ptr); 
}

extern "C" void tuh_midi_umount_cb(uint8_t devAddr, uint8_t unused)
{
  (void)unused;
  umount_cb_fp(devAddr, inst_ptr);
}

extern "C" void tuh_midi_rx_cb(uint8_t devAddr, uint32_t numPackets)
{
  rx_cb_fp(devAddr, numPackets, inst_ptr);
}

