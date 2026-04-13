# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Is

**JourneyOS** is a Qt6/C++17 Android Auto head unit application targeting Raspberry Pi 5 with the official DSI touchscreen. It is a heavily reworked fork of OpenAuto. The desktop builds (macOS, Linux) are used for development only — full hardware functionality (Bluetooth, Wi-Fi, brightness, audio routing) requires Linux.

## Build Commands

```bash
# Desktop (macOS / Linux)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DQT6_INSTALL_DIR="~/Qt/6.x.x/macos"   # macOS only; omit on Linux if Qt is in PATH
cmake --build build -j$(nproc)
```

`aasdk` and `aap_protobuf` must be installed where CMake can find them (or added to `CMAKE_PREFIX_PATH`). Raspberry Pi / Yocto builds are handled by a separate bitbake layer.

There is no unit test framework. Testing is manual on desktop and on-device.

## Two CMake Targets

| Target | Purpose |
|--------|---------|
| `autoapp` | Main GUI application |
| `journeyos-hardware-detect` | Boot-only utility; detects hardware and writes `/run/journeyos/{hardware.json,kms.json,qt.env}`; runs before Weston |

The hardware detect tool is excluded from the `autoapp` GLOB by a CMakeLists filter on `Hardware/Detect/*.cpp` (separate `main`). The `JOURNEYOS_RPI_DETECTION` compile flag gates all RPi-specific code within it.

## Version Bumping

Version is defined in lines 6–12 of the root `CMakeLists.txt` via four variables: `LIBRARY_BUILD_DATE`, `LIBRARY_BUILD_MAJOR_RELEASE`, `LIBRARY_BUILD_MINOR_RELEASE`, `LIBRARY_BUILD_INCREMENTAL`. The display string is assembled from these plus the git short hash. `OPENAUTO_VERSION_STRING` is passed as a compiler define to `autoapp`.

## Architecture Overview

### Entry Point & Startup (`src/autoapp/main.cpp`)

`main.cpp` is the integration hub. It:
1. Initialises libusb and Boost ASIO (8 worker threads: 4 USB + 4 IO)
2. Selects platform-specific backends (audio, Bluetooth, WiFi)
3. Registers ~25 context properties on the QML engine
4. Loads `Journey.qml` as the root window
5. Starts `ProjectionManager` (USB hotplug + TCP:5000 for WiFi projection)

All C++ objects accessible from QML are registered as context properties in `main.cpp` — there is no QML module registration for business logic objects (only for enum types and `HardwareProfile`).

### Platform Backend Selection

Each subsystem has an interface + Linux implementation + Null stub:

| Subsystem | Interface | Linux | Stub |
|-----------|-----------|-------|------|
| Bluetooth | `IBluetoothManager` | `BluetoothHandler` (D-Bus/BlueZ) | `NullBluetoothManager` |
| WiFi monitor | `IWiFiMonitor` | `WifiMonitor` (NetworkManager D-Bus) | `NullWiFiMonitor` |
| WiFi control | `IWiFiController` | `WifiController` (NM D-Bus) | `NullWiFiController` |
| Audio | — | `PulseAudioHandler` | `CoreAudioHandler` / `NullAudioHandler` |

Selection happens at construction time in `main.cpp` via `#ifdef Q_OS_LINUX` etc.

### MVVM Pattern

- **ViewModels** (`src/autoapp/UI/ViewModel/`) own the bridge between config/system and QML. They expose `Q_PROPERTY` with `NOTIFY` signals, and `Q_INVOKABLE` methods for writes. They persist to `QSettings` on every setter call.
- **Monitors** (`src/autoapp/UI/Monitor/`) are read-only system watchers that emit signals on state change.
- **Controllers** (`src/autoapp/UI/Controller/`) are write-only; they perform system operations (NM D-Bus calls, sysfs writes, etc.).
- **Combo models** (`src/autoapp/UI/Combo/`) are `QAbstractListModel` subclasses providing list data to QML ComboBoxes.

### Android Auto Projection Pipeline

```
USB:  ProjectionManager → USBTransport → AndroidAutoSession → ServiceFactory::create()
WiFi: AndroidBluetoothServer (RFCOMM handshake) → ProjectionManager (TCP:5000) → TCPTransport → AndroidAutoSession → ServiceFactory::create()
```

`ServiceFactory::create()` assembles the service list per session: Video, Media/Guidance/Telephony/System audio outputs, Mic input, Sensor, Bluetooth, Input, NavigationStatus, WifiProjection.

All `QtAudioOutput` instances use a dedicated worker thread with async sink creation — always null-guard `audioOutput_` before use.

### Configuration System

Settings are persisted via `QSettings` (INI format, `~/.config/JourneyOS/autoapp.conf` on desktop). Access pattern:

```cpp
configuration->getSettingByName<T>("Group", "Key");
configuration->setSettingByName("Group", "Key", value);
```

Key config groups: `"AndroidAuto"`, `"Wireless"`, `"Bluetooth"`, `"Audio"`, `"Video"`, `"System"`. The `"Wireless"."InterfaceMAC"` key stores the WiFi adapter MAC; `"Wireless"."Interface"` stores the resolved interface name.

### QML Structure

```
Journey.qml           — Root window; global popup layer (Bluetooth, WiFi, Volume, Brightness, AA status)
  MainView.qml        — Home screen; Header + media menus + Settings nav
  AndroidAutoView.qml — Full-screen AA video; home button with 3s fade reveal
  SettingsView.qml    — Tabbed config (Wireless, Bluetooth, Audio, Video, System/OTA)
  BluetoothPopup.qml  — Full-screen modal; adapter selection, paired/unpaired device lists, pairing overlays
  WirelessPopup.qml   — Modal; client/hotspot mode, interface selection, AP scan list
```

`ConfigGate.qml` is a singleton that gates sensitive settings UI: config is hidden when `isSystemManaged && !headUnitMode`. `isSystemManaged` is set in `main.cpp` (true on macOS/Windows, false on Linux RPi).

### Hardware Profile

`journeyos-hardware-detect` runs at boot and writes `/run/journeyos/hardware.json`. The `HardwareProfile` singleton (registered as `import JourneyOS.Hardware 1.0`) reads this file and exposes properties like `HardwareProfile.hasIQAudioDAC`, `.primaryDisplay`, `.hasCANBus` to QML.

## Logging

Category: `Q_LOGGING_CATEGORY(lcAutoapp, "journeyos")` — declare in `.cpp`, `Q_DECLARE_LOGGING_CATEGORY` in headers. Use `qInfo(lcAutoapp) << ...`.

## IDE False Positives

CLion's indexer lacks the Qt include paths. Diagnostics shown in `.hpp`/`.cpp` files are almost always false positives. Trust the CMake build output, not the IDE squiggles.

## Commit Convention

`type: subject` (lowercase). Types: `feat`, `fix`, `chore`, `docs`, `refactor`. PRs target `develop`.

## Branching Strategy

| Branch | Purpose |
|--------|---------|
| `main` | Stable releases — merge here and tag for a release |
| `develop` | Active development — default branch; PRs merge here |
| `feature/<name>` | Feature branches off `develop` |
| `release/<major.minor>` | Optional stabilisation branch before merging to `main` |

Tags use `v<LIBRARY_BUILD_VERSION>` format (e.g. `v4.1.5`). Historical upstream tags on `origin/crankshaft-ng` reflect the pre-fork lineage and should not be moved.