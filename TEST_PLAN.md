# Test Plan: Adding Google Test (gtest) Support

This document outlines the strategy for implementing host-based unit testing for the `openDAQ_LT` project using the Google Test framework.

## Phase 1: Preparation & Scaffolding
1.  **Directory Structure:**
    - `tests/`: Root directory for all test-related files.
    - `tests/unit/`: Specific unit tests for project modules.
    - `tests/mocks/`: Mock implementations of SEGGER headers (`RTOS.h`, `IP.h`, `IP_WEBSOCKET.h`, etc.).
2.  **Build System:** 
    - Create a root `CMakeLists.txt` for the host-based test build.
    - Utilize `FetchContent` to download and integrate `gtest` automatically.
    - Configure include paths to prioritize the `mocks/` directory over the real SEGGER headers.

## Phase 2: Mocking SEGGER Dependencies
Since the project relies heavily on the SEGGER ecosystem (embOS, emNet, emWeb), we must provide mocks or stubs for the following:
- **embOS (RTOS):** `OS_TASK_Terminate`, `OS_MAILBOX_*`, `OS_Delay`, etc.
- **emNet (IP):** `socket`, `bind`, `listen`, `accept`, `send`, `closesocket`, `IP_TCP_*`, `IP_SOCKET_*`.
- **emWeb (Webserver/WebSocket):** `IP_WEBS_WEBSOCKET_AddHook`, `IP_WEBSOCKET_GenerateAcceptKey`.

## Phase 3: Initial Implementation (Pilot Test)
1.  **Target Module:** Start with a self-contained module like `stream_id.c` or `streaming_signals.c`.
2.  **Mock Implementation:** Create a simple mock for `IP.h` used in `stream_id.c`.
3.  **Test Case:** Write a `gtest` case to verify the logic in the target module.

## Phase 4: Expansion
Systematically add tests for:
- `streaming_packet.c`: Verify MessagePack packet construction.
- `streaming_signals.c`: Test signal registration and management.
- `streaming_jsonrpc.c`: Test JSON-RPC parsing and response generation (requires linking `mjson`).
- `streaming_meta.c`: Test MessagePack metadata serialization.

## Key Requirements
- **Host System:** Linux/macOS/Windows with a C++ compiler (GCC/Clang/MSVC).
- **CMake:** Version 3.14 or higher.
- **Third-Party Libraries:** Ensure `mjson` and `mpack` are accessible during the test build.
