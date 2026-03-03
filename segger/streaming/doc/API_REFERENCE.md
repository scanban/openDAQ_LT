# openDAQ Streaming API Reference

This document provides a technical reference for the openDAQ Streaming implementation in `openDAQ_LT`. It covers the layers from network transport to application-level API.

## Architectural Overview

The implementation is structured into four main layers:

1.  **Network Level**: Handles WebSocket/TCP transport and the JSON-RPC Control Channel.
2.  **Protocol Level**: Manages framing (TL packets) and metadata serialization (MessagePack).
3.  **Signal Level**: Manages signal definitions, tables, and subscription states.
4.  **Application Level**: Provides the public API for initialization, configuration, and data transmission.

For a detailed visual representation and step-by-step trace of data movement, see the [Data Flow Documentation](DATA_FLOW.md).

### Layer Diagram

```
+---------------------------------------+
|           Application Layer           | (User code: signals_add_table, streaming_init)
+---------------------------------------+
|             Signal Layer              | (streaming_signals.h: signal_t, signal_table_t)
+---------------------------------------+
|            Protocol Layer             | (streaming_packet.h: tl_packet_t, TL framing)
+---------------------------------------+
|      Network / Transport Layer        | (WebSocket RX, JSON-RPC Control Channel)
+---------------------------------------+
```

---

## 1. Network Level (Transport & Control)

### WebSocket / TCP Transport
The implementation uses SEGGER emWeb for WebSocket handling. 

- `streaming_dispatch_handle(WEBS_OUTPUT *pOutput, void *pConnection)`: Dispatches new WebSocket connections to the streaming task via an OS mailbox.
- `streaming_rx_callback(long Socket, IP_PACKET *pPacket, int code)`: A socket-level callback that handles incoming data on the Data Channel.

### Control Channel (JSON-RPC)
The Control Channel is implemented as JSON-RPC over HTTP, typically on a separate port or URI.

- `streaming_jsonrpc_init(char stream_id[9])`: Initializes the JSON-RPC handler with the current stream ID.
- `streaming_jsonrpc_callback(...)`: The emWeb callback that parses JSON-RPC requests for `subscribe` and `unsubscribe` methods. It validates the `streamId` and updates the signal subscription state.

### Stream Management
- `struct stream`: Represents an active connection.
    - `stream`: Function pointer for sending raw buffers.
    - `streamp`: Function pointer for sending zero-copy TCP packets.
    - `id`: The unique 8-character hex ID for the stream.

---

## 2. Protocol Level (Framing & Metadata)

### Transport Layer Framing
All communication on the Data Channel is framed using a 12-byte header.

- `tl_packet_t`:
    - `type_t packet_type`: `TYPE_DATA (1)` or `TYPE_META (2)`.
    - `uint32_t signal_number`: The internal index of the signal (0 for stream-level meta).
    - `uint32_t payload_size`: Size of the payload in bytes.

- `openDAQ_streaming_send_packet(const struct stream *stream, tl_packet_t *packet)`: Serializes the header and payload into a buffer and sends it through the stream.

### Metadata Serialization (MessagePack)
Metadata messages (signal definitions, availability, etc.) are serialized using `mpack`.

- `streaming_meta.h`:
    - `build_mpack_meta_stream_init`: Generates the "init" message with the `streamId`.
    - `build_mpack_meta_stream_avail`: Generates the "available" message listing all signals.
    - `build_mpack_meta_signal`: Generates the "subscribe" response containing the full signal definition (rules, data type, etc.).

---

## 3. Signal Level (Management)

### Data Structures
- `signal_definition_t`: Contains the static configuration of a signal (name, rule, datatype, delta, etc.).
- `signal_t`: Represents the runtime state of a signal, including its subscription status and reference to its stream.
- `signal_table_t`: Groups signals together (e.g., value, time, and status signals that share the same sampling rate).

### Signal Operations
- `signals_subscribe(const struct stream *stream, const char *signalId)`: Marks a signal as subscribed and triggers the transmission of its metadata.
- `signals_unsubscribe(const struct stream *stream, const char *signalId)`: Marks a signal as unsubscribed.
- `signals_send_all_avail(const struct stream *stream)`: Sends the "available" meta message for all non-hidden signals.

---

## 4. Application Level (Public API)

### Initialization & Startup
- `void streaming_init(struct streaming_callbacks *streaming_cb)`:
    Initializes internal structures, generates a random `streamId`, and sets up the WebSocket hook and JSON-RPC handler.
- `void streaming_start(void)`:
    The main loop for the streaming server. It blocks waiting for connections (via Mailbox or `accept`), initializes the stream, sends initial metadata, and monitors the socket for closure.

### Signal Configuration
- `signal_table_t *signals_add_table(signal_definition_t *def, unsigned int count, const char *table_name)`:
    Registers a group of signals. This must be called before `streaming_start()`.

### Data Serialization
Functions to encode signal data into a user-provided buffer before transmission:

- `openDAQ_streaming_serialize_explicit_signal(...)`: Encodes raw samples.
- `openDAQ_streaming_serialize_constant_signal(...)`: Encodes a single value and its index.
- `openDAQ_streaming_serialize_linear_signal(...)`: Encodes a synchronization point for linear rules (e.g., time).
- `openDAQ_streaming_serialize_implicit_signal(...)`: Encodes a value index without explicit data.

### Data Transmission
- `stream->stream(const struct stream *s, const char *pBuffer, size_t NumBytes)`: Sends a raw buffer (wraps `send()`).
- `stream->streamp(const struct stream *s, void *pPacket)`: Sends a zero-copy TCP packet (wraps `IP_TCP_SendAndFree()`).
