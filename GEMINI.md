# openDAQ_LT

## Project Overview
`openDAQ_LT` is a lightweight, C-based implementation of openDAQ features designed for embedded systems. It provides essential openDAQ functionality like discovery and streaming, specifically optimized for the SEGGER ecosystem (embOS and emNet/emWeb).

### Main Technologies
- **Language:** C
- **RTOS:** SEGGER embOS
- **Networking:** SEGGER emNet, emWeb
- **Data Serialization:** 
    - [mjson](https://github.com/cesanta/mjson) for JSON-RPC configuration.
    - [mpack](https://github.com/ludocode/mpack) for MessagePack metadata serialization.

## Directory Structure
- `segger/`: Contains SEGGER-specific implementations.
    - `discovery/`: Implementation of the openDAQ discovery protocol.
    - `streaming/`: Implementation of the openDAQ streaming protocol, including WebSocket and raw TCP support.
        - `doc/`: Architectural diagrams and documentation.

## Architecture and Key Concepts
- **Static Signal Definition:** Signals must be defined at startup; dynamic addition/removal of signals is not supported.
- **Single Connection:** The current implementation supports only one active streaming connection at a time.
- **Serialization Rules:** Supports explicit, constant, linear, and implicit signal rules for efficient data transmission.
- **Integration:** Designed to run as a dedicated task in an embOS environment.

## Building and Running
The project does not contain a standalone build system (e.g., Makefile or CMake). It is intended to be integrated into a SEGGER Embedded Studio project or a similar embedded build environment.

### Key Integration Steps:
1. **Include Dependencies:** Ensure `embOS`, `emNet`, `emWeb`, `mjson`, and `mpack` are in the include path.
2. **Initialization:** Call `streaming_init()` with appropriate callbacks.
3. **Signal Definition:** Register signals using `signals_add_table()`.
4. **Startup:** Launch the streaming server by calling `streaming_start()` from a dedicated task.

## Development Conventions
- **Naming:** Follows standard C naming conventions (snake_case).
- **Headers:** Most high-level APIs are defined in `streaming_handler.h` and `openDAQ_discovery.h`.
- **Configuration:** Compile-time options are managed in `streaming_config.h` (e.g., ports, buffer sizes, max signals).
- **Licensing:** The codebase contains a mix of Apache 2.0 and HBK-specific license headers.

## Documentation
- `segger/streaming/README.md`: Detailed usage instructions and API reference.
- `segger/streaming/doc/`: Contains sequence and signal diagrams (`.svg`).

## Commit policy
**ALWAYS use COMMIT_STYLE.md** when commiting work in this repository

## Documenting code
**ALWAYS use DOCUMENTATION_GUIDE.md** when documenting code

## Testing 
**ALWAYS use TESTING_POLICY.md** when creating or updating unit or integration tests 
