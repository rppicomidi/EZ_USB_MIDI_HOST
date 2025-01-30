/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 rppicomidi
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
 *
 */

/**
 * This demo program is designed to test the USB MIDI Host driver for a single USB
 * MIDI device connected to the USB Host port or up to 4 devices connected via a
 * USB Hub. It sends to the USB MIDI device(s) the sequence of half-steps from
 * B-flat to D whose note numbers correspond to the transport button LEDs on a Mackie
 * Control compatible control surface. It also prints to a UART serial port console
 * the messages received from each USB MIDI device.
 */

#if !defined(USE_TINYUSB_HOST) || !defined(USE_TINYUSB)
#error "Please use the Menu to select Tools->USB Stack: Adafruit TinyUSB Host"
#else
#warning "All Serial Monitor Output is on Serial1"
#endif

#include "EZ_USB_MIDI_HOST.h"
// USB Host object
Adafruit_USBH_Host USBHost;
USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST

RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, MidiHostSettingsDefault)

/* MIDI IN MESSAGE REPORTING */
static void onMidiError(int8_t errCode)
{
    Serial1.printf("MIDI Errors: %s %s %s\r\n", (errCode & (1UL << ErrorParse)) ? "Parse":"",
        (errCode & (1UL << ErrorActiveSensingTimeout)) ? "Active Sensing Timeout" : "",
        (errCode & (1UL << WarningSplitSysEx)) ? "Split SysEx":"");
}

static void printAddrAndCable()
{
    uint8_t midiDevAddr, cable;
    usbhMIDI.getCurrentReadDevAndCable(midiDevAddr, cable);
    Serial1.printf("[%02d,%02d] ",midiDevAddr, cable);
}

static void onNoteOff(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    Serial1.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
}

static void onNoteOn(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    Serial1.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
}

static void onPolyphonicAftertouch(Channel channel, byte note, byte amount)
{
    printAddrAndCable();
    Serial1.printf("C%u: PAT#%u=%u\r\n", channel, note, amount);
}

static void onControlChange(Channel channel, byte controller, byte value)
{
    printAddrAndCable();
    Serial1.printf("C%u: CC#%u=%u\r\n", channel, controller, value);
}

static void onProgramChange(Channel channel, byte program)
{
    printAddrAndCable();
    Serial1.printf("C%u: Prog=%u\r\n", channel, program);
}

static void onAftertouch(Channel channel, byte value)
{
    printAddrAndCable();
    Serial1.printf("C%u: AT=%u\r\n", channel, value);
}

static void onPitchBend(Channel channel, int value)
{
    printAddrAndCable();
    Serial1.printf("C%u: PB=%d\r\n", channel, value);
}

static void onSysEx(byte * array, unsigned size)
{
    printAddrAndCable();
    Serial1.printf("SysEx:\r\n");
    unsigned multipleOf8 = size/8;
    unsigned remOf8 = size % 8;
    for (unsigned idx=0; idx < multipleOf8; idx++) {
        for (unsigned jdx = 0; jdx < 8; jdx++) {
            Serial1.printf("%02x ", *array++);
        }
        Serial1.printf("\r\n");
    }
    for (unsigned idx = 0; idx < remOf8; idx++) {
        Serial1.printf("%02x ", *array++);
    }
    Serial1.printf("\r\n");
}

static void onSMPTEqf(byte data)
{
    printAddrAndCable();
    uint8_t type = (data >> 4) & 0xF;
    data &= 0xF;    
    static const char* fps[4] = {"24", "25", "30DF", "30ND"};
    switch (type) {
        case 0: Serial1.printf("SMPTE FRM LS %u \r\n", data); break;
        case 1: Serial1.printf("SMPTE FRM MS %u \r\n", data); break;
        case 2: Serial1.printf("SMPTE SEC LS %u \r\n", data); break;
        case 3: Serial1.printf("SMPTE SEC MS %u \r\n", data); break;
        case 4: Serial1.printf("SMPTE MIN LS %u \r\n", data); break;
        case 5: Serial1.printf("SMPTE MIN MS %u \r\n", data); break;
        case 6: Serial1.printf("SMPTE HR LS %u \r\n", data); break;
        case 7:
            Serial1.printf("SMPTE HR MS %u FPS:%s\r\n", data & 0x1, fps[(data >> 1) & 3]);
            break;
        default:
          Serial1.printf("invalid SMPTE data byte %u\r\n", data);
          break;
    }
}

static void onSongPosition(unsigned beats)
{
    printAddrAndCable();
    Serial1.printf("SongP=%u\r\n", beats);
}

static void onSongSelect(byte songnumber)
{
    printAddrAndCable();
    Serial1.printf("SongS#%u\r\n", songnumber);
}

static void onTuneRequest()
{
    printAddrAndCable();
    Serial1.printf("Tune\r\n");
}

static void onMidiClock()
{
    printAddrAndCable();
    Serial1.printf("Clock\r\n");
}

static void onMidiStart()
{
    printAddrAndCable();
    Serial1.printf("Start\r\n");
}

static void onMidiContinue()
{
    printAddrAndCable();
    Serial1.printf("Cont\r\n");
}

static void onMidiStop()
{
    printAddrAndCable();
    Serial1.printf("Stop\r\n");
}

static void onActiveSense()
{
    printAddrAndCable();
    Serial1.printf("ASen\r\n");
}

static void onSystemReset()
{
    printAddrAndCable();
    Serial1.printf("SysRst\r\n");
}

static void onMidiTick()
{
    printAddrAndCable();
    Serial1.printf("Tick\r\n");
}

static void onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow)
{
    if (fifoOverflow)
        Serial1.printf("[%02d,%02d] MIDI IN FIFO overflow\r\n", devAddr, cable);
    else
        Serial1.printf("[%02d,%02d] MIDI IN FIFO error\r\n", devAddr, cable);
}

static void registerMidiInCallbacks(uint8_t midiDevAddr)
{
    uint8_t ncables = usbhMIDI.getNumInCables(midiDevAddr);
    for (uint8_t cable = 0; cable < ncables; cable++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, cable);
        intf->setHandleNoteOff(onNoteOff);                      // 0x80
        intf->setHandleNoteOn(onNoteOn);                        // 0x90
        intf->setHandleAfterTouchPoly(onPolyphonicAftertouch);  // 0xA0
        intf->setHandleControlChange(onControlChange);          // 0xB0
        intf->setHandleProgramChange(onProgramChange);          // 0xC0
        intf->setHandleAfterTouchChannel(onAftertouch);         // 0xD0
        intf->setHandlePitchBend(onPitchBend);                  // 0xE0
        intf->setHandleSystemExclusive(onSysEx);                // 0xF0, 0xF7
        intf->setHandleTimeCodeQuarterFrame(onSMPTEqf);         // 0xF1
        intf->setHandleSongPosition(onSongPosition);            // 0xF2
        intf->setHandleSongSelect(onSongSelect);                // 0xF3
        intf->setHandleTuneRequest(onTuneRequest);              // 0xF6
        intf->setHandleClock(onMidiClock);                      // 0xF8
        // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
        intf->setHandleTick(onMidiTick);                        // 0xF9
        intf->setHandleStart(onMidiStart);                      // 0xFA
        intf->setHandleContinue(onMidiContinue);                // 0xFB
        intf->setHandleStop(onMidiStop);                        // 0xFC
        intf->setHandleActiveSensing(onActiveSense);            // 0xFE
        intf->setHandleSystemReset(onSystemReset);              // 0xFF
        intf->setHandleError(onMidiError);
    }
    auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
    if (dev == nullptr)
        return;
    dev->setOnMidiInWriteFail(onMidiInWriteFail);
}

static void unregisterMidiInCallbacks(uint8_t midiDevAddr)
{
    uint8_t ncables = usbhMIDI.getNumInCables(midiDevAddr);
    for (uint8_t cable = 0; cable < ncables; cable++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, cable);
        if (intf == nullptr)
            return;
        intf->disconnectCallbackFromType(NoteOn);
        intf->disconnectCallbackFromType(NoteOff);
        intf->disconnectCallbackFromType(AfterTouchPoly);
        intf->disconnectCallbackFromType(ControlChange);
        intf->disconnectCallbackFromType(ProgramChange);
        intf->disconnectCallbackFromType(AfterTouchChannel);
        intf->disconnectCallbackFromType(PitchBend);
        intf->disconnectCallbackFromType(SystemExclusive);
        intf->disconnectCallbackFromType(TimeCodeQuarterFrame);
        intf->disconnectCallbackFromType(SongPosition);
        intf->disconnectCallbackFromType(SongSelect);
        intf->disconnectCallbackFromType(TuneRequest);
        intf->disconnectCallbackFromType(Clock);
        // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
        intf->disconnectCallbackFromType(Tick);
        intf->disconnectCallbackFromType(Start);
        intf->disconnectCallbackFromType(Continue);
        intf->disconnectCallbackFromType(Stop);
        intf->disconnectCallbackFromType(ActiveSensing);
        intf->disconnectCallbackFromType(SystemReset);
        intf->setHandleError(nullptr);
    }
    auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
    if (dev == nullptr)
        return;
    dev->setOnMidiInWriteFail(nullptr);
}

/* CONNECTION MANAGEMENT */
static void listConnectedDevices()
{
    Serial1.printf("Dev  VID:PID  Product Name[Manufacter]{serial string}\r\n");
    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
        if (dev) {
            Serial1.printf("%02u  %04x:%04x %s[%s]{%s}\r\n",midiDevAddr, dev->getVID(), dev->getPID(),
                dev->getProductStr(), dev->getManufacturerStr(), dev->getSerialString());
        }
    }
}
static void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables)
{
    Serial1.printf("MIDI device at address %u has %u IN cables and %u OUT cables\r\n", devAddr, nInCables, nOutCables);
    registerMidiInCallbacks(devAddr);
    listConnectedDevices();
}

static void onMIDIdisconnect(uint8_t devAddr)
{
    Serial1.printf("MIDI device at address %u unplugged\r\n", devAddr);
    unregisterMidiInCallbacks(devAddr);
    // Note that listConnectedDevices() will still list the just unplugged
    //  device as connected until this function returns
    listConnectedDevices();
}


/* MAIN LOOP FUNCTIONS */

static void blinkLED(void)
{
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;

    static bool ledState = false;
    if ( millis() - startMs < intervalMs)
        return;
    startMs += intervalMs;

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH:LOW); 
}

static void sendNextNote()
{
    static uint8_t firstNote = 0x5b; // Mackie Control rewind
    static uint8_t lastNote = 0x5f; // Mackie Control stop
    static uint8_t offNote = lastNote;
    static uint8_t onNote = firstNote;
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;
    if (millis() - startMs < intervalMs)
        return; // not enough time
    startMs += intervalMs;
    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, usbhMIDI.getNumOutCables(midiDevAddr)-1);
        if (intf == nullptr)
            continue; // not connected
        intf->sendNoteOn(offNote, 0, 1);
        intf->sendNoteOn(onNote, 0x7f, 1);
        
    }
    if (++offNote > lastNote)
        offNote = firstNote;
    if (++onNote > lastNote)
        onNote = firstNote;
}


/* APPLICATION STARTS HERE */
void setup()
{
  Serial1.begin(115200);

  while(!Serial1);   // wait for serial port
  pinMode(LED_BUILTIN, OUTPUT);
  usbhMIDI.begin(&USBHost, 0, onMIDIconnect, onMIDIdisconnect);
  Serial1.println("EZ_USB_MIDI_HOST Example");
}

void loop() {    
    // Update the USB Host
    USBHost.task();

    // Handle any incoming data; triggers MIDI IN callbacks
    usbhMIDI.readAll();
    
    // Do other processing that might generate pending MIDI OUT data
    sendNextNote();
    
    // Tell the USB Host to send as much pending MIDI OUT data as possible
    usbhMIDI.writeFlushAll();
    
    // Do other non-USB host processing
    blinkLED();
}

