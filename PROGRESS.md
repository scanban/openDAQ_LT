# Progress Report: openDAQ_LT Testing Infrastructure

## Status Overview
- **Phase 1: Preparation & Scaffolding** - **COMPLETE**
- **Phase 2: Mocking SEGGER Dependencies** - **COMPLETE**
- **Phase 3: Initial Implementation (Pilot Test)** - **COMPLETE**
- **Phase 4: Expansion** - **IN PROGRESS**

---

## Detailed Progress

### Phase 1: Preparation & Scaffolding
- [x] Established `tests/` directory structure (`unit/`, `mocks/`).
- [x] Created root `CMakeLists.txt` with Google Test integration via `FetchContent`.
- [x] Configured include paths for mocks and source modules.
- [x] Added `mjson` and `mpack` as external dependencies in the test build.

### Phase 2: Mocking SEGGER Dependencies
- [x] Implemented mock headers in `tests/mocks/`:
    - `RTOS.h`: embOS tasks, mutexes, and mailboxes.
    - `IP.h`: emNet sockets (BSD-compatible), MDNS, and delayed execution.
    - `IP_Webserver.h`: emWeb HTTP hooks and data handling.
    - `IP_WEBSOCKET.h`: WebSocket hooks and key generation.
- [x] Created `tests/mocks/mocks.cpp` with C-linkage stubs for all mocked functions.
- [x] Added `extern "C"` wrappers to all SEGGER mock headers and core project headers (`stream_id.h`, `streaming_handler.h`, `streaming_signals.h`).

### Phase 3: Initial Implementation (Pilot Test)
- [x] Target: `stream_id.c`.
- [x] Implemented `tests/unit/stream_id_test.cpp`.
- [x] Verified basic stream allocation and cleanup logic.

### Phase 4: Expansion
- [x] **streaming_signals.c**:
    - [x] Fixed missing `string.h` include.
    - [x] Fixed pointer dereference bug in `signals_subscribe`.
    - [x] Corrected `signals_add_signal` visibility (removed `static`).
    - [x] Added counter resets to `signals_init` to support test isolation.
    - [x] Implemented and passed `tests/unit/streaming_signals_test.cpp`.
- [ ] **streaming_packet.c**: Pending.
- [ ] **streaming_jsonrpc.c**: Pending.
- [ ] **streaming_meta.c**: Pending.

---

## Next Steps
1. Implement unit tests for `streaming_packet.c` to verify MessagePack packet construction.
2. Implement unit tests for `streaming_meta.c` for metadata serialization.
3. Implement unit tests for `streaming_jsonrpc.c` (requires integration with `mjson` mock/logic).
