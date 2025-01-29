# EZ_USB_MIDI_HOST
This README file contains the design notes and limitations of the
C++ library code that lets Arduino sketches and C/C++ programs
use the Arduino MIDI Library with the usb_midi_host library and
the TinyUSB Library (Arduino sketches require the Adafruit TinyUSB
Library).

The Arduino MIDI Library provides an API that performs most
of the MIDI byte-level parsing and encoding that applications
need. Applications such as synthsizers and MIDI controllers
are probably easier to implement using that library than by
using the usb_midi_host library alone. Interface bridge
devices that require no MIDI parsing or MIDI filter applications
that require a great deal of parsing anyway probably do
not benefit from it.

The code in this project should run on any TinyUSB supported
processor that runs the USB MIDI Host Library.

This documentation is for version 2.0.0 of this library or
later. The previous versions used a different API.

# Adding this library to your project
## C/C++ Programs
First, you must install the [usb_midi_host](https://github.com/rppicomidi/usb_midi_host) library in your file
system at the same directory level as this project. That is,
if this library source is installed in directory `EZ_USB_MIDI_HOST`,
then the directory of the usb_midi_host library must be
`EZ_USB_MIDI_HOST/../usb_midi_host`. You must also make sure
you have the pico-sdk installed correctly, that the TinyUSB library
is up to date, and if your hardware requires it, the Pico_PIO_USB
library is installed. See the Building C/C++ applications section
of the usb_midi_host [README](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md)
for more information.
Finally, you must install the [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library)
at the same directory level as this library and the usb_midi_host library.

In the project's `CMakeLists.txt` `target_link_libraries` section, 
add the `EZ_USB_MIDI_HOST` library. The library interface
in cmake will pull in other library dependencies it needs.
See the examples in `examples/C-Code` for examples. If you are
using the Pico PIO USB Library to implement the USB Host,
see the `EZ_USB_MIDI_HOST_PIO_example` for other details.

## Arduino
First, use the Library Manager to install this library and install all of
its dependencies (Adafruit TinyUSB Arduino Library, the Arduino MIDI Library,
the usb_midi_host Library). Next, if your hardware requires it, install the
Pico_PIO_USB library.

Adding `#include "EZ_USB_MIDI_HOST.h"` to your sketch should be sufficient
to integrate your Arduino sketch with this library and all of its dependencies.
If you are using the Pico PIO USB Library to implement the host, you must
also add `#include "pio_usb.h"` for `#include "EZ_USB_MIDI_HOST.h"`.
See the `EZ_USB_MIDI_HOST_PIO_example` for other details.

# EZ_USB_MIDI_HOST Library Design
The Arduino MIDI Library has a Transport
class interface that expects one Transport object per bidirectional MIDI
stream. USB MIDI devices are complex because each one can support up to
16 unique bidirectional MIDI streams on so-called virtual MIDI cables.
If you connect a USB hub to the host port, you can have up to the
maximum supported number of hub ports devices connected. Finally, USB
MIDI devices can be connected and disconnected while the program is running.

To make this complexity more manageable, this library provides the
following software components layered as follows:

|              |                            |
| ------------ | -------------------------- |
|              | application                |
|              | EZ_USB_MIDI_HOST           |
|              | EZ_USB_MIDI_HOST_Device    |
| MIDI Library | EZ_USB_MIDI_HOST_Transport |
| TinyUSB      | usb_midi_host_app_driver   |

The application interacts with a single `EZ_USB_MIDI_HOST` object.
The `EZ_USB_MIDI_HOST` object has as many `EZ_USB_MIDI_HOST_Device`
objects as there are hub ports. Each device has a configurable
number of `MIDI Library` bidirectional streams; each stream has
a `EZ_USB_MIDI_HOST_Transport` interface. Each`EZ_USB_MIDI_HOST_Transport` object interacts with the
`TinyUSB` library supplemented by the `usb_midi_host_app_driver`.

# Writing Applications
To create an instance of the EZ_USB_MIDI_HOST class for your
program, you should use the `RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE()`
macro. The first argument is name of the EZ_USB_MIDI_HOST object
the macro defines. The second argument is the settings class to
apply to the object. See the CONFIGURATION and IMPLEMENTATION DETAILS
section for more information.

In practice, assuming the name of the EZ_USB_MIDI_HOST instance
the code creates is `usbhMIDI`, then main loop's body looks like this
```
        // Update the USB Host
        USBHost.task(); // Arduino, comment out or delete for C++
        // tuh_task(); // C++, comment out or delete for Arduino

        // Handle any incoming data; triggers MIDI IN callbacks
        // The MIDI IN callbacks should call usbhMIDI.getCurrentReadDevAndCable()
        // to know from which device the MIDI message was sent
        usbhMIDI.readAll();
    
        // Do other processing that might generate pending MIDI OUT data
        // For example, insert code here that sends Note On/Off messages.
    
        // Tell the USB Host to send as much pending MIDI OUT data as possible
        usbhMIDI.writeFlushAll();
    
        // Do other non-USB host processing
        // For example blink an LED
```
Note that this loop must call `usbhMIDI.writeFlushAll()` after generating
about 16 USB MIDI packets or else the transmitter buffers will overflow.

The library keeps track of what devices are connected. A way 
for the application to know if a device is connected is to attempt to
get a pointer to the device object based on one of the possible
valid USB device addresses: 1 to `RPPICOMIDI_TUH_MIDI_MAX_DEV`,
inclusive. For example:
```
    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
        if (dev != nullptr) {
            // do stuff with the connected device
        }
    }
```

However, the application needs to register callbacks on device connection
and unregister them on device disconnection. The application may
have to perform other tasks on routing and unrouting tasks on
connect and disconnect events, too. The library supports a `ConnectCallback` function
and a `DisconnectCallback` function to support this things.
Note that the device is not fully disconnected until the `DisconnectCallback` returns. 
The application can also use these callbacks to track what devices are connected.

Please see the example programs for instructions on how to initialize the library
in your application. Note that the `begin()` function will call `tuh_init()`; if
you use this library in an application that uses the TinyUSB for purposes other
than MIDI, please be sure not to call `tusb_init()` or `tuh_init()` functions
directly. C++ programs should call the TinyUSB `board_init()` function before calling `begin()`.

# EXAMPLE PROGRAMS

## Hardware
See the Hardware section of the `usb_midi_host`
[REAMDE](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md)
for the different hardware configurations.

## Software
There are 2 Arduino examples and 2 C/C++ code examples in folders
named `arduino` and `C-Code`, respectively. The examples in folders named
`EZ_USB_MIDI_HOST_example` use the native USB host hardware. The examples
in `EZ_USB_MIDI_HOST_PIO_example` use the PIO for the USB host hardware.
These programs all support USB MIDI devices either directly connected
to the USB host port or connected to the host port up to 4 devices through
a hub. All programs do the same thing.
- play a 5 note sequence  (B-flat to D by half step) on all the highest
  number virtual MIDI cable on all connected MIDI devices. If your MIDI
  device has Mackie Control transport button LEDs, this will light all
  5 transport buttons in sequence.
- print out every MIDI message it receives on cable 0. The numbers in
  brackets are the connected MIDI device number and the virtual cable number.

### C/C++ Examples
To build the rp2040 C/C++ examples, install the pico-sdk and all required
libraries in your build environment.

Then enter these commands
```
cd [insert example program directory name here]
mkdir build
cd build
cmake ..
make
```
This will generate the `.uf2` file that you can load to
your rp2040 board in the normal way.

### Arduino Examples
To run the Arduino examples, in the Arduino IDE, use the library
manager to install this library and all of its dependencies. If
your hardware requires it, install the Pico_PIO_USB library.
Next select File->Examples->EZ_USB_MIDI_HOST->arduino->[example program name].
A new sketch window will open up. See the Building Arduino Applications section 
of the usb_midi_host
[README](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md) for
for setting up the board parameters. Note that native rp2040 hardware example
directs Serial output to the Serial1 port, which is rp2040 UART0.

# LIBRARY CONFIGURATION and IMPLEMENTATION DETAILS
Because the Arduino IDE's build system does not support configuring
libraries using preprocessor macros and constants defined in files
in the sketch directory, version 2.0.0 and later of this library
redefines the API for using this class to make it possible for
applications to configure the library without editing the library
configuration files directly.

The EZ_USB_MIDI_HOST class, as well as the classes it uses, are
implemented as template classes that depend on a settings class. The
settings class must be the `MidiHostSettingsDefault` struct defined in `EZ_USB_MIDI_HOST_Config.h` or a subclass of it. The
`MidiHostSettingsDefault` struct is itself a subclass of the Arduino
MIDI Library's `DefaultSettings` class. If you need to change one of
the settings in EZ_USB_MIDI_HOST` or in the Arduino MIDI Library, then
please define a subclass of `MidiHostSettingsDefault` and pass this
class as an argument to the `RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE`
macro. You may overload any field the settings structure.

For example, let's say that your application needs to send and
receive System Exclusive messages that are 146 bytes long (excluding
the 0xF0 start of SysEx message and 0xF7 end of SysEx message bytes).
The default settings in `MidiHostSettingsDefault` assume that the
longest SysEx message is 128 bytes long. Your application would
have to create a new settings class to make buffers large enough
to for the underlying usb_midi_host library to handle the longer
SysEx messages, and it would have to tell the Arduino MIDI library
that it needs to handle 128 bytes SysEx payloads.

```
struct MidiHostSettingsDefault : public MIDI_NAMESPACE::DefaultSettings
{
    static const unsigned SysExMaxSize = 146; // for MIDI Library
    static const unsigned MidiRxBufsize = RPPICOMIDI_EZ_USB_MIDI_HOST_GET_BUFSIZE(SysExMaxSize);
    static const unsigned MidiTxBufsize = RPPICOMIDI_EZ_USB_MIDI_HOST_GET_BUFSIZE(SysExMaxSize);
};
```
