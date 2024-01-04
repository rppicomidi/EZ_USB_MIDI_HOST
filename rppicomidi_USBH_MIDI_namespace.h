/**
 * @file rppicomidi_USBH_MIDI_namespace.h
 * @brief Namespace definition helper for rppicomidi_USBH_MIDI Arduino wrapper
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

#define RPPICOMIDI_USBH_MIDI_NAMESPACE                  rppicomidi_usbh_midi
#define BEGIN_RPPICOMIDI_USBH_MIDI_NAMESPACE            namespace RPPICOMIDI_USBH_MIDI_NAMESPACE {
#define END_RPPICOMIDI_USBH_MIDI_NAMESPACE              }

#define USING_NAMESPACE_USBH_MIDI            using namespace RPPICOMIDI_USBH_MIDI_NAMESPACE;

BEGIN_RPPICOMIDI_USBH_MIDI_NAMESPACE

END_RPPICOMIDI_USBH_MIDI_NAMESPACE
