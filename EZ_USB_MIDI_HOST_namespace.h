/**
 * @file EZ_USB_MIDI_HOST_namespace.h
 * @brief Namespace definition helper for EZ_USB_MIDI_HOST Arduino wrapper
 *
 * This file follows the pattern used in the Arduino MIDI driver.
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
#pragma once

#define EZ_USB_MIDI_HOST_NAMESPACE                  ez_usb_midihost
#define BEGIN_EZ_USB_MIDI_HOST_NAMESPACE            namespace EZ_USB_MIDI_HOST_NAMESPACE {
#define END_EZ_USB_MIDI_HOST_NAMESPACE              }

#define USING_NAMESPACE_EZ_USB_MIDI_HOST            using namespace EZ_USB_MIDI_HOST_NAMESPACE;

BEGIN_EZ_USB_MIDI_HOST_NAMESPACE

END_EZ_USB_MIDI_HOST_NAMESPACE
