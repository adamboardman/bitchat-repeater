A simple bitchat repeater
=========================

The bitchat system uses these UUID's:

```
PRIMARY_SERVICE, F47B5E2D-4A9E-4C5A-9B3F-8E1D2C3A4B5C
CHARACTERISTIC, A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D, READ | WRITE | DYNAMIC,
```

The idea is that you set these just inside the BLE range within your building, to allow bitchat to be used when there
are not enough devices about to join everyone together.

They should be generally sleeping, only waking up when there are messages to send along. This means that the heartbeat
led blinking only works when woken up by there being messages to deliver. To tell if a repeater node is functioning you
can drag a piece of metal across the bottom quarter of one side to join pins 23(GND) & 24(GP18) which will fire off a
message into the network 'Sill Alive', and also cause some blinking of the LED. Avoid the top half as you'd be shorting
the power and the exit to USB for update flashing which can be done by carefully pulling pin 34(GP28) to ground.

Building
========

Note I've only tested this with the PicoW, as the cheaper device it would seem more likely to be used dotted about the
place as repeaters.

Command Line
------------

To build on the command line you could type something along the lines of:

```
bitchat-repeater$ mkdir build
bitchat-repeater$ cd build
bitchat-repeater/build$ cmake .. -DPICO_SDK_PATH=~/pico-pi/pico-sdk -DPICOTOOL_FETCH_FROM_GIT_PATH=~/pico-pi/picotool
bitchat-repeater/build$ make
```

CLion
-----

If using CLion you'll need to set 'CMake options' to something along the lines of the following:

```
-DPICO_SDK_PATH=~/pico-pi/pico-sdk -DPICOTOOL_FETCH_FROM_GIT_PATH=~/pico-pi/picotool
```

For step through debugging 'Edit configurations...' to then add '+' and pick 'Open OCD Download & Run', select your
target 'bitchat_repeater', Executable binary should be 'bitchat_repeater' or 'bitchat_repeater.elf' you may need to
select this directly from the cmake-build-debug directory, bundled GDB is fine, Board config file should be a new file
you'll probably need to create in your local copy of the raspberry pi openocd 'openocd/tcl/board/pico.cfg'. I set
Download to 'Always', the rest of the defaults were fine.

I created a new openocd/tcl/board/pico.cfg containing:

```
# SPDX-License-Identifier: GPL-2.0-or-later
# Attempt to make clion happy

source [find interface/cmsis-dap.cfg]
adapter speed 5000

set CHIPNAME rp2040
source [find target/rp2040.cfg]
```

I also had to set the openocd to my locally built version, 'Settings' -> 'Build, Execution, Deployment' -> 'Embedded
Development' -> Open OCD location: '/usr/local/bin/openocd'. Though it populated itself with that value once CLion
noticed it was missing.

Debugging
---------

When running attached to a linux computer via the USB port you can view what is going on with:

```
sudo minicom -D /dev/ttyACM0 -b 115200
```

or

```
screen -L /dev/ttyACM0 115200
```

Bluetooth debugging
-------------------

Useful tools for debugging BTLE things from a linux desktop:

Interactive tool:

```
bluetoothctl 
```

Commands such as 'scan on', 'connect XX:XX:XX:XX:XX:XX' can be useful to report info about a specific device. For
example both my android devices merrily report their names, even if their ble is using a random address.

Scan for BTLE devices:

```
sudo hcitool lescan
```

Interactively talk to BTLE device:

```
gatttool -b  XX:...:XX -I
[XX:...:XX][LE]> connect
[XX:...:XX][LE]> primary
[XX:...:XX][LE]> char-desc
```

* primary - lists UIDs supported
* char-desc lists handles and UIDs

Bitchat
=======

Problems with offering a repeater
---------------------------------

We can scan for other local devices offering the bitchat service and then connect to them. We don't know the time so if
we attempt to send any original messages they are ignored as they are more than 5 minutes (more like 55+ years) old. But
without us doing an Announce if we initiated the connection they don't include us as a valid peer so we don't get sent
any messages. So we have to wait for them to connect to us first then nab the time from a packet, then next time we see
a new device we can originate the connection. We also have no way of determining if a new random bt address that we find
on a scan is the same as an existing connection until after we've connected and recieved an announce packet.

Compression is it zlib or lz4? (zlib it is)
-------------------------------------------

I've only got Android hardware to test compatibility with and having first implemented LZ4 decompression as mentioned in
the documentation, I found it failed to decompress messages, I then switched to zlib. However, I now see in the
BinaryProtocols.swift file it says both 'zlib compression for broad compatibility on Apple platforms' and 'Bit 2: Is
compressed (LZ4 compression applied)' so I can see how implementors not doing any cross-platform testing can be doing
different things. Though the actual swift code says 'COMPRESSION_ZLIB', so it seems most likely that 'bitchatz-cpp' is
the outlier project on its own with LZ4. We don't actually need to decompress messages as we are just passing them along
but for testing and verification purposes it is useful to know the data going about the place.

Security Concerns with Bitchat Protocol
---------------------------------------

If you use a ble browsing app you can read the device name freely to any connection, this can contain the device model
name or the users personalised name. This remains stable across the changes in random UUID's. Activating the 'triple
click to clear the history' does as described, but it immediately starts announcing its new anonNNNN name on the same
random bluetooth address as the previous name was reported under. Then some minutes later the UUID is re-randomised and
we keep announcing ourselves. If you're worried about an adversary who might gain something from the packet size
analysis then they gain much more from just advertising acceptance of the service UUID and listening to the
Announce/NoiseIDAnnounce messages, requesting device names and of course reading the public messages in the general
chat which all get posted.

The BLE connections used are unsecured so there is no encryption of the packets, so any sufficiently motivated adversary
could read all the public messages entirely passively without advertising the service, the protocol is public so there
is no point adding padding whilst reporting that in the plain text part of the message. There could be an argument
keeping the padding of noise messages, but care should be taken to pad to make the encasing packet the desired size not
the encrypted one, it will result in stable packet sizes but will lead to premature fragmentation.

The apps should clearly state that there is no notion of anonymity or secrecy within the public chat, and support the
creation of secure chat rooms via QR-Code or similar, something along the lines of how it is handled in Meshtastic would
seem sensible. The secure person to person chat using the noise protocol could be man in the middle attacked if both
devices are only ever able to see the middle device and never each other.

More peculiarities with the padding
-----------------------------------

The Android code uses a single byte to indicate the length of padding added (this would lead you to believe to expect
padding added in the range 0-255 bytes). But then lists padding options as: 256, 512, 1024, 2048. So if a message is say
520 bytes long before padding it will be padded upto the next value which is 1024, resulting in padding 504 bytes, but
then there is a check to say don't bother padding > 255 so such a message would actually get no padding, though if it
was between 768 bytes and 1024 it would have gotten padded. See above argument, padding after encryption is pointless.

BLE has the concept of negotiating an MTU, the android app completely ignores this and has a fixed 512 bytes message
size around which it fragments. In fact if you create a single character encrypted message, this results in an 80 byte
message that is then put into a noise encrypted message packet and padded up to 256 which is then put as the payload for
a new noise type message which of course adds a fresh copy of the header information and then pads that upto the next
threshold so now its 512 bytes being sent for one byte of actual user data.

This of course means that when we first tried to parse such a message we got 252 bytes, and it flags as corrupt when the
payload length of 276 is unreadable. Please note that with a reported mtu of 255 the probable actual max message size is
252 bytes, as the att header needs adding. For simplicity, I just upped the bitchat-repeater MTU to 517 to match the
Android app, but realistically we should cope with other apps having different settings.

A more sensible list of paddings could be something along the lines of:

* 128, 233, 491, 747, 1003, 1259, 1515, 1771, 2027.

And only be applied to a message if it's going to be encrypted.

Message format drops binary
---------------------------

The payload of message types for some reason switches to text mode for the entire message not just the actual text part
First the message ID is commonly written out in a hex printed human readable form with '-' between parts, then the
optional sender_peer_id part is hex printed values. No one reads these parts so it would seem to make sense to save a
few bytes and keep with the rest of the format and go back to binary. For the message ID this isn't a breaking change as
the length is given first so it would just be a shorter block. The sender_peer_id part would be a breaking change.

Suggestions
-----------

Before this format gets more widely adopted we could just roll a v2 of the format and tweak a few bits and bobs. One
idea would be to archive type: 4 messages, and add two new types of message. The first one we could call Public Message
which would be just be a binary version of the current message format, with no padding added. The second we could call
Private Message, and this one would have identical payload content to the just mentioned Public Message but with the
addition of a reduced padding. Private Messages would then be Noise encrypted. Client applications can then read all
three types of message with the same input code, and just tailor their output depending on the version supported by the
recipient. The version supported by each connection should then be tracked and appropriate message format used for
output, this would also allow for per-connection mtu size checking and appropriate fragmentation.