# JourneyOS

**JourneyOS** is a heavily reworked and rebranded fork of [OpenAuto](https://github.com/f1xpl/openauto), an Android Auto™ head unit emulator. It builds on the work of the original OpenAuto author and the [CrankShaft](https://github.com/opencardev/crankshaft) project, to whom we owe significant thanks.

This is not a minor fork. The UI has been redesigned from the ground up using Qt Quick/QML, the architecture has been substantially refactored, and the project is being actively developed toward a production-quality embedded head unit for Raspberry Pi.

> **Android Auto™ is a registered trademark of Google Inc. This software is not certified by Google Inc. It is provided for research and development purposes. Use at your own risk. Do not use while driving.**

---

## Status

Active development — targeting v4.1 for Raspberry Pi 5. The project compiles and runs on macOS for development purposes, but full functionality is Linux/Raspberry Pi only at this stage.

---

## Attribution

- **OpenAuto** — original Android Auto head unit emulator by [Michal Szwaj (f1x.studio)](https://github.com/f1xpl/openauto). Licensed under GNU GPLv3.
- **CrankShaft** — Raspberry Pi-focused OpenAuto distribution by the [OpenCarDev](https://github.com/opencardev/crankshaft) team.

JourneyOS is derived from these works and is distributed under the same GNU GPLv3 licence.

---

## Features

- Android Auto wired and wireless (Wi-Fi) projection
- 480p / 720p / 1080p video at 30 or 60 FPS
- Audio playback across all channels — Media, System, Speech
- Audio input for voice commands
- Touchscreen and button input
- Bluetooth pairing and device management
- Wi-Fi hotspot and client configuration
- PulseAudio volume control and device selection
- Screen brightness control (Raspberry Pi LCD backlight)
- Automatic USB device hotplug detection
- Settings lockdown mode for managed/embedded deployments
- Raspberry Pi 5 with official touchscreen as primary target

---

## Dependencies

| Dependency | Version | Notes |
|------------|---------|-------|
| CMake | ≥ 3.16 | Build system |
| C++ standard | C++17 | Required |
| Qt | 6.x | Core, Gui, Qml, Quick, Bluetooth, DBus, Network, Multimedia, Widgets |
| Boost | Any recent | `system`, `log` components; dynamic linking |
| OpenSSL | Any recent | SSL + Crypto |
| libusb | 1.0 | USB device communication |
| FFmpeg | Any recent | Video decode |
| taglib | Any recent | Audio metadata |
| aasdk | — | Android Auto SDK (must be built separately) |
| aap_protobuf | — | Android Auto protobuf definitions (must be built separately) |
| PulseAudio | — | **Linux only** — audio routing |
| libgps (GPSD) | — | **Linux only** — GPS sensor data |
| blkid | — | **Linux only** — block device identification |

---

## Building

### Raspberry Pi / Yocto

Build is handled by the Yocto layer. All dependencies are provided by the sysroot. Cross-compilation is detected automatically and Qt Design Studio network components are disabled accordingly.

### Desktop (macOS / Linux)

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DQT6_INSTALL_DIR="~/Qt/6.x.x/macos"   # macOS — omit on Linux if Qt is in PATH

cmake --build build -j$(nproc)
```

`aasdk` and `aap_protobuf` must be installed to a location CMake can find, or their install prefixes added to `CMAKE_PREFIX_PATH`.

---

## Roadmap

The roadmap below covers near-term targets only. Scope beyond that is being finalised and will be published when confirmed.

### v4.1 — Raspberry Pi 5 Foundation *(in progress)*

| Status | Item |
|--------|------|
| ✅ | Android Auto wireless projection |
| ✅ | Yocto build |
| ✅ | Raspberry Pi LCD support (dynamic resolution, fullscreen, touch) |
| ✅ | Home button returns to main menu |
| ✅ | Volume control |
| ✅ | Brightness control |
| ✅ | All audio channels work and are configurable |
| ✅ | All status indicators correct |
| ✅ | Power on / restart |
| ✅ | System managed settings (locked in embedded mode) |
| 🔲 | Bluetooth pairing and HFP/A2DP configuration |
| 🔲 | Wi-Fi hotspot / client configuration |
| 🔲 | "Hey Google" voice activation without freeze |
| 🔲 | Video sizing — full screen and display-size aware |
| 🔲 | OTA updates via RAUC |

### v4.2 — Polish *(planned)*

- Time of day synchronisation from Android Auto
- Dark / Night mode
- Multiple audio quality advertisement to Android Auto
- Navigation audio prompts when Android Auto is not in the foreground

---

## Licence

GNU General Public License v3.0 — see [LICENSE](LICENSE) for the full text.

Original work Copyright © 2018 f1x.studio (Michal Szwaj).
JourneyOS modifications Copyright © the JourneyOS contributors.