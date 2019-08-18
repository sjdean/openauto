
# OpenAuto

### Support project
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=R4HXE5ESDR4U4)

**OpenAuto Pro version** provides features like brightness control, volume control, support of Kodi and integration with the Raspberry PI OS (Raspbian Desktop). [See how it works at OpenAuto Pro overview video](https://www.youtube.com/watch?v=9sTOMI1qTiA)

If you are looking for the car power supply with ignition state detection [go to the Blue Wave Studio page](https://bluewavestudio.io/index.php/bluewave-shop)

For support of other platforms please contact me at f1xstudiopl@gmail.com

### Community
[![Join the chat at https://gitter.im/publiclab/publiclab](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/openauto_androidauto/Lobby)

### Description
OpenAuto is an AndroidAuto(tm) headunit emulator based on aasdk library and Qt libraries. Main goal is to run this application on the RaspberryPI 3 board computer smoothly.

[See demo video](https://www.youtube.com/watch?v=k9tKRqIkQs8)

### Supported functionalities
 - 480p, 720p and 1080p with 30 or 60 FPS
 - RaspberryPI 3 hardware acceleration support to decode video stream (up to 1080p@60!)
 - Audio playback from all audio channels (Media, System and Speech)
 - Audio input for voice commands
 - Touchscreen and buttons input
 - Bluetooth
 - Automatic launch after device hotplug
 - Automatic detection of connected Android devices
 - Wireless (WiFi) mode via head unit server (must be enabled in hidden developer settings)
 - User-friendly settings

### Supported platforms

 - Linux
 - RaspberryPI 3
 - Windows

### License
GNU GPLv3

Copyrights (c) 2018 f1x.studio (Michal Szwaj)

*AndroidAuto is registered trademark of Google Inc.*

### Used software
 - [aasdk](https://github.com/f1xpl/aasdk)
 - [Boost libraries](http://www.boost.org/)
 - [Qt libraries](https://www.qt.io/)
 - [CMake](https://cmake.org/)
 - [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/playback.html)
 - Broadcom ilclient from RaspberryPI 3 firmware
 - OpenMAX IL API

### CANBUS Extensions
This Branch is a Work In Progress.

A separate module to be provided shortly collects data over Ethernet UDP Broadcast and using a JSON file, will map the CANBUS data to car features - ie accelerator position, parking brake, steering wheel position, speed, rpm etc.

These updates will allow be able to take a snapshot of the data for feeding information back to Android Auto and OpenAuto will be updated to allow registration of a car for reporting data into Firebase.

Currently reporting of telemetry information into Firebase will be over a WiFi connection (ie HotSpot), but we should see if we can send the data to the phone and allow the phone to store and send the data itself as well as provide registration functions.

Next steps for this project is to ensure that this compiles and OpenAuto can see the data. There are some issues with includes and not being ablt to get to the underlying properties, this could be my development environment or may be issues with the structure.

Any help gloriously accepted.

### Remarks
**This software is not certified by Google Inc. It is created for R&D purposes and may not work as expected by the original authors. Do not use while driving. You use this software at your own risk.**

