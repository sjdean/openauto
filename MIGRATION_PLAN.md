# v5.0 Migration Plan: Boost → Qt

Generated: 2026-04-09
Branch: v4.2 → targets v5.0 / `develop`

---

## What This Is

A staged plan to remove Boost entirely from both **aasdk** (`aasdk3/aasdk`) and **openauto** (this repo), re-engineering all async, threading, timer, TCP, and logging infrastructure to use Qt 6 natively. The Yocto layer (`meta-headunit`) is updated last once the build dependencies are clean.

---

## 1a. Communication Architecture Overview

### The Stack

```
Android Phone
     │  USB (AOAP) or WiFi (TCP:5000)
     ▼
┌─────────────────────────────────────────────────────┐
│  Transport Layer                                    │
│  USBTransport / TCPTransport  (ITransport)          │
│  ├── USBEndpoint  →  libusb bulkTransfer async      │
│  └── TCPEndpoint  →  boost::asio async_read/write   │
└──────────────────┬──────────────────────────────────┘
                   │ raw bytes (Promise<Data>)
┌──────────────────▼──────────────────────────────────┐
│  Frame Layer                                        │
│  MessageInStream / MessageOutStream                 │
│  ├── Frame header (4 bytes): ChannelId, FrameType,  │
│  │   EncryptionType, MessageType                    │
│  ├── Optional size word (FIRST frames > 16KB)       │
│  └── Payload ≤ 16KB per frame (fragmented as BULK / │
│       FIRST / MIDDLE / LAST)                        │
└──────────────────┬──────────────────────────────────┘
                   │ frames + Cryptor (OpenSSL memory BIOs)
┌──────────────────▼──────────────────────────────────┐
│  Messenger Layer                                    │
│  Demultiplexes frames → per-channel queues          │
│  receiveStrand_ (deserialise) + sendStrand_ (queue) │
└──────────────────┬──────────────────────────────────┘
                   │ Message::Pointer (ChannelId tagged)
┌──────────────────▼──────────────────────────────────┐
│  Channel / Service Layer (19 ChannelIds)            │
│  ControlServiceChannel  ─── all others inherit      │
│  │                          Channel base            │
│  ▼  Session establishment order:                    │
│  1. sendVersionRequest()  (PLAIN, SPECIFIC)         │
│  2. phone → VERSION_RESPONSE                        │
│  3. sendHandshake() — SSL bytes in                  │
│     MESSAGE_ENCAPSULATED_SSL frames (PLAIN)         │
│  4. ← handshake response from phone                 │
│  5. doHandshake() loop until SSL_do_handshake() = 1 │
│  6. sendAuthComplete() — signals session active     │
│  7. phone → SERVICE_DISCOVERY_REQUEST               │
│  8. sendServiceDiscoveryResponse() (ENCRYPTED)      │
│  9. per-channel service objects activate            │
└──────────────────┬──────────────────────────────────┘
                   │ Qt signals → openauto UI/services
┌──────────────────▼──────────────────────────────────┐
│  openauto Service layer (ServiceFactory)            │
│  VideoMediaSinkService → QML video surface          │
│  AudioMediaSinkService → PulseAudio / CoreAudio     │
│  InputSourceService    → QML touch/key events       │
│  SensorService         → GPS, speed, night mode     │
│  BluetoothService      → BlueZ D-Bus                │
│  … 14 others                                        │
└─────────────────────────────────────────────────────┘
```

### Threading Model (current, Boost-based)

```
main thread          Qt event loop  (QML, UI, signals)
io_service thread 0  io_service.run()  ─┐
io_service thread 1  io_service.run()   │  handle all ASIO async ops
io_service thread 2  io_service.run()   │
io_service thread 3  io_service.run()  ─┘
usb thread 0-3       libusb_handle_events_timeout_completed()
```

Serialisation across the 4 IO threads is provided by **18 strands in openauto** + **~12 strands in aasdk** (Transport×2, Messenger×2, InStream×1, OutStream×1, each Channel×1, USBHub×1, USBEndpoint×1, AccessoryModeQuery×1…).

### Key Async Pattern (io::Promise)

```cpp
// aasdk's hand-rolled continuation system
auto promise = io::Promise<common::Data>::defer(receiveStrand_);
tcpEndpoint_->receive(buffer, promise);
promise->then(
  [this, self = shared_from_this()](auto data) { /* on strand */ },
  [this, self = shared_from_this()](auto err)  { /* on strand */ }
);
```

SSL uses **raw OpenSSL memory BIOs**, not `boost::asio::ssl`. The handshake bytes travel inside `MESSAGE_ENCAPSULATED_SSL` frames on the control channel. Transport sockets are never SSL sockets; encryption/decryption happens inside `Cryptor` within `MessageInStream`/`MessageOutStream`.

---

## 1. Speed Benefit Analysis

The gains are real but are primarily about **size, integration, and developer velocity** — not raw throughput.

| Area | Current (Boost) | Qt replacement | Gain |
|---|---|---|---|
| **Binary / image size** | `libboost-system.so` + `libboost-log.so` pulled into sysroot | Already-linked `libQt6Network.so`; no new dependency | Medium — smaller Yocto image, faster cold start on RPi5 |
| **Build time** | Boost headers among the slowest in C++ to compile | Qt headers far lighter; no Boost template sprawl | Large developer-experience win |
| **Logging** | `boost::log` pipeline with thread-safe sinks; not trivially disabled | `Q_LOGGING_CATEGORY` compiles to zero overhead in release when category is off | Small but real in the message receive loop |
| **Dual event loop boundary** | **Two** separate event loops: ASIO (4 threads) + Qt (main thread). Every AA event crosses this boundary | **One** event loop system; AA services are QObjects, signals stay inside Qt | **Most meaningful runtime gain** — eliminates the handoff latency on video/audio frames |
| **Timer dispatch** | `deadline_timer` + `posix_time` conversion overhead | `QTimer` uses `timerfd` on Linux natively | Minor; timers here are low-frequency |
| **TCP** | ASIO proactor → io_service queue → strand dispatch | `QTcpSocket` via epoll/kqueue; `readyRead` signal | Comparable throughput; AA WiFi is bandwidth-bound, not dispatch-bound |
| **Memory** | 4 ASIO threads + Qt main thread; strands ~48 bytes each | QObject thread affinity; QObject ~80 bytes overhead | Roughly neutral |
| **USB path** | libusb polling threads, unchanged | Same — libusb is a C API regardless | No change |

**Bottom line**: The most concrete wins are removing the dual-event-loop boundary, image/binary size reduction, and faster incremental builds. Raw video/audio throughput is hardware and protocol bound, not dispatcher bound.

---

## 2. High-Level Steps Overview

Two codebases change — **aasdk** (protocol library) and **openauto** (application) — plus the **Yocto recipes**. The dependency order requires aasdk interface changes first where the public API is affected; openauto follows.

```
A. aasdk
   A1. Logging (Boost.Log → Qt logging categories)
   A2. boost::noncopyable → Q_DISABLE_COPY
   A3. boost::system::error_code → std::error_code
   A4. TCPWrapper/TCPEndpoint → QTcpSocket-based endpoint
   A5. io::Promise → QFuture / QPromise  ← largest aasdk change
   A6. strands → QObject thread affinity + Qt::QueuedConnection
   A7. Remove io_service parameter from all constructors
   A8. CMakeLists: remove Boost, confirm Qt6::Network

B. openauto
   B1. Logging (remove residual Boost log includes)
   B2. boost::noncopyable → Q_DISABLE_COPY on InputDevice
   B3. boost::algorithm::hex → QByteArray::toHex() (or delete unused include)
   B4. boost::circular_buffer → std::vector ring buffer in SequentialBuffer
   B5. boost::posix_time → std::chrono timer durations
   B6. deadline_timer → QTimer (10 services + Pinger)
   B7. TCP acceptor/socket in ProjectionManager → QTcpServer / QTcpSocket
   B8. Remove strands from all 18 service headers (superseded by A6)
   B9. Remove io_service from all constructors and main.cpp
   B10. CMakeLists: remove Boost

C. meta-headunit (Yocto)
   C1. Remove `boost` from aasdk_git.bb DEPENDS
   C2. Remove `boost` from openauto_git.bb DEPENDS
   C3. Verify no RDEPENDS on libboost-* in image
```

A5 and A6 are the load-bearing work. Everything else is mechanical. A5 changes aasdk's public API (the `io::Promise` types appear in every `ITransport`, `IMessenger`, and `IChannel` interface), which triggers corresponding updates in openauto's service implementations.

---

## 3. Full Staged Plan

Each stage is a discrete, buildable, testable unit of work. Stages within a phase can sometimes be parallelised; stages across phases are sequential.

---

### Phase 0 — Groundwork (no functional change)

**Stage 0.1 — Add Qt Network to aasdk CMake (don't use it yet)**
- `aasdk/CMakeLists.txt`: add `find_package(Qt6 REQUIRED COMPONENTS Core Network)` alongside existing Boost find
- Add `Qt6::Core Qt6::Network` to `target_link_libraries` alongside existing libs
- Verify build still passes with both present
- *Files*: `aasdk/CMakeLists.txt`

**Stage 0.2 — Add `Q_LOGGING_CATEGORY` infrastructure to aasdk Log.hpp**
- Add `#include <QLoggingCategory>` and declare `Q_DECLARE_LOGGING_CATEGORY(lcAasdk)` in `Log.hpp`
- Define `Q_LOGGING_CATEGORY(lcAasdk, "aasdk")` in a new `src/Common/Log.cpp`
- Keep `AASDK_LOG` macro working with a second Qt-based definition alongside the Boost one
- *Files*: `include/aasdk/Common/Log.hpp`, new `src/Common/Log.cpp`, `aasdk/CMakeLists.txt`

---

### Phase 1 — Mechanical replacements (both repos, no functional change)

**Stage 1.1 — aasdk: Boost.Log → Qt logging**
- Replace `AASDK_LOG(severity)` macro: `BOOST_LOG_TRIVIAL(severity) << "[AASDK] "` → `qCDebug(lcAasdk)` / `qCInfo` / `qCWarning` / `qCCritical`
- New macro: `#define AASDK_LOG(severity) qC##severity(lcAasdk)`
- Remove `#include <boost/log/...>` from `Log.hpp`
- `aasdk/CMakeLists.txt`: remove `Boost::log` from `target_link_libraries`; keep `Boost::system` for now
- *No other files change*

**Stage 1.2 — aasdk: boost::noncopyable → Q_DISABLE_COPY**
- Replace `boost::noncopyable` base class with `Q_DISABLE_COPY(ClassName)` in class body for:
  `Transport.hpp`, `Messenger.hpp`, `MessageInStream.hpp`, `MessageOutStream.hpp`,
  `USBEndpoint.hpp`, `USBHub.hpp`, `AccessoryModeQuery.hpp`, `AccessoryModeQueryChain.hpp`,
  `ConnectedAccessoriesEnumerator.hpp`
- Remove `#include <boost/noncopyable.hpp>` from each

**Stage 1.3 — openauto: boost::noncopyable → Q_DISABLE_COPY**
- `include/f1x/openauto/autoapp/Projection/InputDevice.hpp`: replace `boost::noncopyable` base with `Q_DISABLE_COPY(InputDevice)` in class body

**Stage 1.4 — openauto: remove unused boost::algorithm::hex include**
- `src/autoapp/Bootstrap/AndroidBluetoothServer.cpp`: confirm no actual `boost::algorithm` call exists (code already uses `std::hex`/`std::setfill`), then delete the include

**Stage 1.5 — openauto: boost::posix_time → std::chrono**
- `Pinger.cpp`: `boost::posix_time::milliseconds(duration_)` → `std::chrono::milliseconds(duration_)`
- `SensorService.cpp`: `boost::posix_time::milliseconds(250)` → `std::chrono::milliseconds(250)`
- These are bridge values until Stage 2.1 replaces the timer type itself

---

### Phase 2 — Timers and TCP in openauto

**Stage 2.1 — openauto: deadline_timer → QTimer (all 10 services)**

For each service header with `boost::asio::deadline_timer timer_`:
- Replace `boost::asio::deadline_timer timer_` with `QTimer timer_`
- Remove `ioService` constructor parameter (QTimer uses owning object's thread event loop)
- Replace the `expires_from_now` + `async_wait(strand_.wrap(...))` pattern:
  ```cpp
  // before
  timer_.expires_from_now(std::chrono::milliseconds(250));
  timer_.async_wait(strand_.wrap(std::bind(&SensorService::sensorPolling, shared_from_this())));

  // after
  timer_.setSingleShot(true);
  timer_.setInterval(250);
  connect(&timer_, &QTimer::timeout, this, &SensorService::sensorPolling);
  timer_.start();
  ```
- Replace `timer_.cancel()` → `timer_.stop()`
- Services: `Pinger`, `SensorService`, `GenericNotificationService`, `MediaBrowserService`,
  `MediaPlaybackStatusService`, `NavigationStatusService`, `PhoneStatusService`,
  `RadioService`, `VendorExtensionService`, `WifiProjectionService`

**Stage 2.2 — openauto: TCP acceptor → QTcpServer in ProjectionManager**
- `ProjectionManager.hpp`: replace `boost::asio::ip::tcp::acceptor acceptor_` with `QTcpServer tcpServer_`
- Remove `boost::asio::ip::tcp::socket` parameter from `handleNewClient`
- `ProjectionManager.cpp`:
  - Constructor: `tcpServer_.listen(QHostAddress::Any, 5000)` replaces ASIO acceptor init
  - Replace `startServerSocket()` + `async_accept` + `handleNewClient(socket, err)`:
    ```cpp
    connect(&tcpServer_, &QTcpServer::newConnection, this, &ProjectionManager::handleNewClient);

    void ProjectionManager::handleNewClient() {
        auto *socket = tcpServer_.nextPendingConnection();
        // ... existing session creation logic
    }
    ```

**Stage 2.3 — openauto: remove strand_ from ProjectionManager**
- Remove `boost::asio::io_service::strand strand_` member
- Remove all `strand_.dispatch(...)` calls — TCP events now arrive via Qt signals on the correct thread
- Remove `boost/asio/io_service.hpp` include from `ProjectionManager.hpp`

At this point ProjectionManager has zero Boost dependency.

---

### Phase 3 — aasdk TCP layer (QTcpSocket replaces Boost TCP)

**Stage 3.1 — aasdk: new QtTCPEndpoint implementing ITCPEndpoint**
- Create `include/aasdk/TCP/QtTCPEndpoint.hpp` and `src/TCP/QtTCPEndpoint.cpp`
- `QtTCPEndpoint` is a `QObject` wrapping `QTcpSocket`
- Implement `send(DataConstBuffer, Promise::Pointer)` and `receive(DataBuffer, Promise::Pointer)`:
  - send: `socket_.write(...)`, connect `bytesWritten` signal to resolve the promise
  - receive: on `readyRead` when enough bytes are available, resolve the promise
- Keep the same `ITCPEndpoint` interface — no public API change yet
- `stop()`: `socket_.disconnectFromHost()`

**Stage 3.2 — aasdk: remove boost::system::error_code from ITCPWrapper**
- `ITCPWrapper.hpp`: change `Handler` typedef from `std::function<void(const boost::system::error_code&, size_t)>` to `std::function<void(std::error_code, size_t)>`
- Update `TCPWrapper.cpp` accordingly
- Update `TCPEndpoint.cpp` `asyncOperationHandler`: `boost::system::error_code` → `std::error_code`
- Verify `Boost::system` can be removed from aasdk `target_link_libraries` at this point

**Stage 3.3 — aasdk: wire QtTCPEndpoint as default, remove old TCP classes**
- Update `TCPTransport` constructor to accept `QtTCPEndpoint` (or keep `ITCPEndpoint` interface)
- Update openauto `SessionFactory` / `ProjectionManager` to construct `QtTCPEndpoint`
- Delete `TCPEndpoint.hpp/.cpp`, `ITCPWrapper.hpp`, `TCPWrapper.hpp/.cpp`

---

### Phase 4 — io::Promise → QFuture/QPromise (aasdk — largest change)

This phase changes aasdk's public API. All callers in openauto must be updated in step, or a compatibility shim can bridge temporarily.

**Stage 4.1 — Audit: map every Promise specialisation**
*(Research commit — document findings, no code change)*
- List every `io::Promise<T, E>` specialisation in use across aasdk and openauto
- Map each to its Qt equivalent:
  - One-shot result → `QFuture<T>` with `.then()` / `.onFailed()`
  - Streaming / repeated → signal/slot
  - Completion notification → `QFuture<void>`
- Identify non-void error types requiring `QFuture<Expected<T,E>>` or custom approach

**Stage 4.2 — Create thin `QtPromise<T>` adaptor (temporary bridge)**
- `include/aasdk/IO/QtPromise.hpp`: a `QObject`-based wrapper accepting `resolve(T)` / `reject(Error)`, exposing a `QFuture<T>`
- Internally uses Qt 6 `QPromise<T>`
- The `defer(strand)` semantic becomes: continuation runs on the QObject's event-loop thread
- This shim allows stages 4.3–4.5 to migrate one layer at a time without a big-bang

**Stage 4.3 — Migrate Transport promises**
- `ITransport::ReceivePromise` → `QFuture<common::Data>`
- `ITransport::SendPromise` → `QFuture<void>`
- Update `USBTransport`, `TCPTransport`, `Transport` base class
- Update `MessageInStream` and `MessageOutStream` which consume these

**Stage 4.4 — Migrate Messenger promises**
- `IMessenger::ReceivePromise` → `QFuture<Message::Pointer>`
- `IMessenger::SendPromise` → `QFuture<void>`
- Update `Messenger`, `MessageInStream`, `MessageOutStream`
- Update all Channel implementations calling `messenger_->enqueueReceive()` / `enqueueSend()`

**Stage 4.5 — Migrate Channel promises**
- `Channel::SendPromise` → `QFuture<void>`
- All 19 channel service headers and implementations
- Update openauto service wrappers calling into these channels

**Stage 4.6 — Remove io::Promise, IOContextWrapper, PromiseLink**
- Delete `include/aasdk/IO/Promise.hpp`, `IOContextWrapper.hpp`, `PromiseLink.hpp`
- Delete `src/IO/IOContextWrapper.cpp`
- Remove from CMakeLists source lists

---

### Phase 5 — Strands → QObject thread affinity (aasdk)

After Phase 4, async operations use `QFuture`. The strand pattern is replaced by Qt's thread-affinity model.

**Stage 5.1 — Make Transport a QObject**
- `Transport` inherits `QObject` (drop `boost::noncopyable`, add `Q_OBJECT`)
- Move `Transport` to a dedicated worker `QThread`
- Remove `receiveStrand_` and `sendStrand_` members
- `receive()` and `send()` called from any thread use `QMetaObject::invokeMethod(this, ..., Qt::QueuedConnection)` to marshal to Transport's thread

**Stage 5.2 — Make Messenger a QObject**
- Same pattern as 5.1
- Remove `receiveStrand_` and `sendStrand_`
- `enqueueReceive` / `enqueueSend` marshal to Messenger's thread via `Qt::QueuedConnection`

**Stage 5.3a — Make MessageInStream / MessageOutStream QObjects (mechanical)**
- `strand_` removed; they live on the same thread as Messenger

**Stage 5.3b — MessageInStream: redesign frame demux and per-channel buffer logic**

This is a targeted rework of the frame reassembly logic, independent of the threading changes
above. The existing implementation survived patching but carries implicit ordering assumptions
that caused widespread frame rejection when the protocol delivered interleaved or out-of-sequence
frame types.

The Android Auto frame stream is not ordered per-channel. Legal sequences include:

```
BULK BULK BULK
FIRST MIDDLE LAST
FIRST BULK MIDDLE BULK LAST      ← bulk from other channels mid-stream
BULK FIRST BULK BULK MIDDLE LAST
```

**Current problems to fix:**
- The state machine (`thisFrameType_`, `isValidFrame_`, `frameSize_`, `message_`) is shared mutable
  state across chained callbacks. Stale state from a failed frame persists into the next.
- `interleavedPromise_` as a parallel promise is a patch over the real issue: a single-stream
  handler was never the right model for a multi-channel demux.
- No explicit per-channel sequence validation — a MIDDLE or LAST with no prior FIRST for that
  channel has undefined behaviour.
- `messageBuffer_` cleanup on error/session teardown is not guaranteed.
- No channel ID bounds checking at the Messenger demux level.

**Proposed redesign — explicit per-channel accumulator:**

```cpp
struct PartialMessage {
    ChannelId channelId;
    EncryptionType encryptionType;
    MessageType messageType;
    common::Data payload;
};

QHash<ChannelId, PartialMessage> inProgressMessages_;
QSet<ChannelId> knownChannelIds_;   // populated at service discovery
```

Processing rules (no implicit state carry-over):

| Frame type | Action |
|---|---|
| `BULK` | Decrypt if needed → emit complete message immediately. Buffer untouched. |
| `FIRST` | Validate no existing entry for this ChannelId (log + discard stale if present). Create new `PartialMessage`. |
| `MIDDLE` | Validate entry exists for ChannelId. Append payload. |
| `LAST` | Validate entry exists. Append. Move out of map → emit complete message. |
| Unknown ChannelId | Log + discard frame. Never insert into map. |
| MIDDLE/LAST with no FIRST | Log + discard. Do not attempt recovery. |

Additional hardening:
- `resetChannel(ChannelId)`: removes any in-progress entry; called on channel close or error.
- `resetAll()`: called on session teardown — clears entire `inProgressMessages_` map.
- Max accumulated payload guard per channel (e.g. 512 KB); reject + reset channel if exceeded,
  guarding against a malformed FIRST that claims an enormous total size.
- `knownChannelIds_` populated at service discovery time; frames for unknown channels are
  discarded rather than buffered.

`MessageOutStream` does not require equivalent changes — output framing is fully deterministic.

**Stage 5.4 — Make each Channel a QObject**
- `Channel` base: inherits `QObject`, removes `boost::asio::io_service::strand &strand_` from constructor
- All 19 channel subclasses updated

**Stage 5.5 — Make USB classes QObjects**
- `USBHub`, `USBEndpoint`, `AccessoryModeQuery`, `AccessoryModeQueryChain`, `ConnectedAccessoriesEnumerator`
- `strand_` members removed; thread affinity provides the same serialisation guarantee
- USB polling threads remain as `std::thread` — libusb is a C API

---

### Phase 6 — Remove io_service from all signatures (aasdk + openauto)

After Phase 5 no component needs an `io_service`; Qt's threading is self-contained.

**Stage 6.1 — Remove io_service parameter from aasdk constructors**
- `USBTransport(io_service, aoapDevice)` → `USBTransport(aoapDevice)`
- `TCPTransport(io_service, tcpEndpoint)` → `TCPTransport(tcpEndpoint)`
- `Messenger(io_service, inStream, outStream)` → `Messenger(inStream, outStream)`
- `MessageInStream(io_service, transport, cryptor)` → `MessageInStream(transport, cryptor)`
- `MessageOutStream(io_service, transport, cryptor)` → `MessageOutStream(transport, cryptor)`
- All channel constructors: remove `strand&` parameter
- `USBHub`, `USBEndpoint`: remove `io_service&`

**Stage 6.2 — Remove io_service from openauto constructors**
- All 30+ service classes that accept `io_service&` and `strand_` — remove those parameters
- `SessionFactory`, `ServiceFactory`, `ProjectionManager` constructors updated
- `AndroidAutoSession`: remove `io_service` member

**Stage 6.3 — Remove io_service from main.cpp**
- Delete `boost::asio::io_service ioService;` and `boost::asio::io_service::work work(ioService);`
- Delete `startIOServiceWorkers()` and its 4 threads
- Delete `startUSBWorkers()` — USB threads managed by `USBHub`'s own `QThread`
- `std::vector<std::thread> threadPool` → managed internally by each Qt component

---

### Phase 7 — Remaining openauto cleanups

**Stage 7.1 — Remove remaining 18 strand members from openauto services**
- After Phase 6 all strand members in openauto service headers are dead code
- Remove `boost::asio::io_service::strand strand_` from all 18 service headers
- Remove all `strand_.dispatch(...)` calls — replace with direct calls (safe because each service is a QObject on a known thread)

**Stage 7.2 — Replace boost::circular_buffer in SequentialBuffer**
- `include/f1x/openauto/autoapp/Projection/SequentialBuffer.hpp`
- Replace `boost::circular_buffer<char> m_buffer` with a `std::vector<char>` ring buffer (head/tail indices, capacity = 4 MB)
- Remove `#include <boost/circular_buffer.hpp>`
- This is the last non-ASIO Boost header in openauto

---

### Phase 8 — CMakeLists and build system cleanup

**Stage 8.1 — aasdk CMakeLists final cleanup**
- Remove `find_package(Boost REQUIRED COMPONENTS system log)`
- Remove `Boost::system` and `Boost::log` from `target_link_libraries`
- Remove `BOOST_ALL_DYN_LINK` compile definition if present
- Update CPack DEB deps: remove `libboost-system-dev libboost-log-dev`
- Verify `Qt6::Core Qt6::Network` are correctly exported in `aasdkConfig.cmake`

**Stage 8.2 — openauto CMakeLists final cleanup**
- Remove `find_package(Boost REQUIRED COMPONENTS system log)` (CMakeLists.txt line 132)
- Remove `Boost::system` and `Boost::log` from `target_link_libraries` (lines 325–326)
- Remove `-DBOOST_ALL_DYN_LINK` compiler definition (line 261)

---

### Phase 9 — Yocto / meta-headunit

**Stage 9.1 — aasdk recipe**
- `recipes-apps/aasdk/aasdk_git.bb`: remove `boost` from `DEPENDS`

**Stage 9.2 — openauto recipe**
- `recipes-apps/openauto/openauto_git.bb`: remove `boost` from `DEPENDS`
- `qtbase` (providing QtNetwork) is already present — no new DEPENDS needed

**Stage 9.3 — Full image build verification**
- `bitbake core-image-weston`
- Confirm no `libboost-*.so` present in rootfs
- Confirm `autoapp` starts and Android Auto session establishes over USB and WiFi

---

## Dependency Map

```
Phase 0  ─ groundwork (CMake + logging infra)
Phase 1  ─ mechanical replacements (parallel across both repos)
Phase 2  ─ openauto timers + TCP (can start after Phase 1)
Phase 3  ─ aasdk TCP (parallel to Phase 2)
Phase 4  ─ io::Promise → QFuture (needs Phase 3)
Phase 5  ─ strands → QObject affinity (needs Phase 4)
Phase 6  ─ io_service removal (needs Phase 5)
Phase 7  ─ openauto leftovers (needs Phase 6)
Phase 8  ─ CMakeLists cleanup (needs Phase 7)
Phase 9  ─ Yocto recipes (needs Phase 8)
```

Phases 1, 2, and 3 can proceed in parallel across the two repos. Everything from Phase 4 onward is sequential because each phase changes the API that the next phase depends on.

---

## Boost Inventory Summary

### aasdk (`aasdk3/aasdk`)

| Boost component | Location | Qt replacement |
|---|---|---|
| `boost::asio::io_service` | All constructors | Remove (QObject thread affinity) |
| `boost::asio::io_service::strand` | Transport×2, Messenger×2, InStream, OutStream, each Channel, USBHub, USBEndpoint, AccessoryModeQuery | Remove (QObject on QThread) |
| `boost::log` / `BOOST_LOG_TRIVIAL` | `Common/Log.hpp` + every `.cpp` | `Q_LOGGING_CATEGORY` |
| `boost::system::error_code` | `TCPEndpoint`, `ITCPWrapper` | `std::error_code` |
| `boost::noncopyable` | ~10 headers | `Q_DISABLE_COPY()` |
| `boost::asio::ip::tcp::socket` | `TCPEndpoint`, `TCPWrapper` | `QTcpSocket` |
| `Boost::system` + `Boost::log` | `CMakeLists.txt` | `Qt6::Core Qt6::Network` |

### openauto

| Boost component | Location | Qt replacement |
|---|---|---|
| `boost::asio::io_service` | `main.cpp`, all service constructors | Remove (Phase 6) |
| `boost::asio::io_service::strand` | 18 service headers | Remove (Phase 7) |
| `boost::asio::deadline_timer` (10×) | Pinger + 9 services | `QTimer` |
| `boost::asio::ip::tcp::acceptor` | `ProjectionManager.hpp` | `QTcpServer` |
| `boost::asio::ip::tcp::socket` | `ProjectionManager.cpp` | `QTcpSocket` |
| `boost::system::error_code` | `ProjectionManager.hpp`, `Pinger.hpp` | `std::error_code` / Qt socket error |
| `boost::posix_time::milliseconds` | `Pinger.cpp`, `SensorService.cpp` | `std::chrono::milliseconds` |
| `boost::circular_buffer<char>` | `SequentialBuffer.hpp` | `std::vector<char>` ring buffer |
| `boost::algorithm::hex` | `AndroidBluetoothServer.cpp` | Delete (unused) |
| `boost::noncopyable` | `InputDevice.hpp` | `Q_DISABLE_COPY()` |
| `Boost::system` + `Boost::log` | `CMakeLists.txt` | Already uses Qt; remove Boost lines |