# rppicomidi MIDI HOST LIBRARY WRAPPER
This README file contains the design notes and limitations of the
C++ wrapper code that lets Arduino sketches and C/C++ programs
use the Arduino MIDI Library with the USB MIDI Host library and
the Adafruit TinyUSB Library.

The Arduino MIDI Library provides an API that performs most
of the MIDI byte-level parsing and encoding that applications
need. Applications such as synthsizers and MIDI controllers
are probably easier to implement using that library than by
using the USB MIDI Host Library alone. Interface bridge
devices that require no MIDI parsing or MIDI filter applications
that require a great deal of parsing anyway probably do
not benefit from it.

The code in this project should run on any TinyUSB supported
processor that runs the USB MIDI Host Library.

# Adding this library to your project
## C/C++ Programs
First, you must install the [usb_midi_host]() library in your file
system at the same directory level as this project. That is,
if this library source is installed in directory `rppicomidi_USBH_MIDI`,
then the directory of the usb_midi_host library must be
`rppicomidi_USBH_MIDI/../usb_midi_host`. You must also make sure
you have the pico-sdk installed correctly, that the TinyUSB library
is up to date, and if your hardware requires it, the Pico_PIO_USB
library is installed. See the Building C/C++ applications section
of the `usb_midi_host` [REAMDE](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md)
for more information.
Finally, you must install the [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library)
at the same directory level as this library and the usb_midi_host library.

In the project's `CMakeLists.txt` `target_link_libraries` section, 
add the `rppicomidi_USBH_MIDI` library instead. The library interface
in cmake will pull in other library dependencies it needs.
See the examples in `examples/C-Code` for examples.

## Arduino
First, use the Library Manager to install this library and install all of
its dependencies (Adafruit TinyUSB Arduino Library, the Arduino MIDI Library,
the usb_midi_host Library). Next, if your hardware requires it, install the
Pico_PIO_USB library.

Adding `#include "rppicomidi_TUSB_MIDI.h"` to your sketch should be sufficient
to integrate your Arduino sketch with this library and all of its dependencies.

# Arduino MIDI Library Wrapper Design
The Arduino MIDI Library has a Transport
class interface that expects one Transport object per bidirectional MIDI
stream. USB MIDI devices are complex because each one can support up to
16 unique bidirectional MIDI streams on so-called virtual MIDI cables.
If you connect a USB hub to the host port, you can have up to the
maximum supported number of hub ports devices connected. Finally, USB
MIDI devices can be connected and disconnected while the program is running.

To make this complexity more manageable, this wrapper provides the
following software components layered as follows:

|              |                           |
| ------------ | ------------------------- |
|              | application               |
|              | rppicomidi_USBH           |
|              | rppicomidi_USBH_Device    |
| MIDI Library | rppicomidi_USBH_Transport |
| TinyUSB      | usb_midi_host_app_driver  |

The application interacts with a single `rppicomidi_USBH` object.
The `rppicomidi_USBH` object has as many `rppicomidi_USBH_Device`
objects as there are hub ports. Each device has a configurable
number of `MIDI Library` bidirectional streams; each stream has
a `rppicomidi_USBH_Transport` interface. Each`rppicomidi_USBH_Transport` object interacts with the
`TinyUSB` library supplimented by the `usb_midi_host_app_driver`.

# Writing Applications
In practice, the code for main loop's body looks like this
```
        // Update the USB Host
        USBHost.task(); // Arduino, comment out or delete for C++
        // tuh_task(); // C++, comment out or delete for Arduino

        // Handle any incoming data; triggers MIDI IN callbacks
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

The application still has to keep track of what devices are connected.
To do this, it should implement the `ConnectCallback` function to
record the USB device address of the attached MIDI device and to
register MIDI IN callbacks for the supported messages. It should also
implement the `DisconnectCallback` function to unregister the MIDI IN
callbacks associated with the disconnected device address and
to forget the device address of the unplugged MIDI device.

All the setup section of the main application has to do is register
the ConnectCallback, DisconnectCallback, and call the library's `begin()`
function.

# EXAMPLE PROGRAMS

## Hardware
See the Hardware section of the `usb_midi_host`
[REAMDE](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md)
for the different hardware configurations.

## Software
Each of the 4 example program does the same thing:
- play a 5 note sequence on MIDI cable 0
- print out every MIDI message it receives on cable 0.

The only difference among the example programs is C/C++ vs.
Arduino, and native RP2040 USB host hardware vs. Pico_PIO_USB
USB host hardware. 

### C/C++ Examples
To build the C/C++ examples, install the pico-sdk all required
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
Next select File->Examples->rppicomidi_USBH_MIDI->arduino->[example program name].
A new sketch window will open up. See the Building Arduino Applications section 
of the usb_midi_host
[REAMDE](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md) for
for setting up the board parameters. Note that native rp2040 hardware example
directs Serial output to the Serial1 port, which is rp2040 UART0.

# CONFIGURATION and IMPLEMENTATION DETAILS
See the usb_midi_host library [REAMDE](https://github.com/rppicomidi/usb_midi_host/blob/main/README.md).