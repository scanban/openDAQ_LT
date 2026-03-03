#include <gtest/gtest.h>
#include "streaming_packet.h"
#include "streaming_signals.h"
#include "SEGGER_UTIL.h"
#include <cstring>
#include <vector>

// Mock for stream_send
static int last_sent_bytes = 0;
static char last_sent_buffer[2048];

static int mock_stream_send(const struct stream *s, const char *pBuffer, size_t NumBytes) {
    (void)s;
    last_sent_bytes = (int)NumBytes;
    if (NumBytes <= sizeof(last_sent_buffer)) {
        memcpy(last_sent_buffer, pBuffer, NumBytes);
    }
    return (int)NumBytes;
}

class StreamingPacketTest : public ::testing::Test {
protected:
    void SetUp() override {
        last_sent_bytes = 0;
        memset(last_sent_buffer, 0, sizeof(last_sent_buffer));
        signals_init();
    }

    void VerifyHeader(const uint8_t* data, uint32_t signal_no, uint32_t type, uint32_t payload_size) {
        const uint8_t* tl_ptr = data;

#ifdef WEBSOCKET_STREAMING
        // WebSocket Header
        uint8_t fin_binary = tl_ptr[0];
        EXPECT_EQ(fin_binary & 0x80, 0x80); // FIN bit
        EXPECT_EQ(fin_binary & 0x0F, 0x02); // Binary frame

        uint8_t payload_len_7bit = tl_ptr[1];
        uint32_t tl_header_size = (payload_size > 255) ? 8 : 4;
        uint32_t websocket_payload_size = tl_header_size + payload_size;

        if (websocket_payload_size < 126) {
            EXPECT_EQ(payload_len_7bit, websocket_payload_size);
            tl_ptr += 2;
        } else {
            EXPECT_EQ(payload_len_7bit, 126);
            uint16_t extended_payload_len = (tl_ptr[2] << 8) | tl_ptr[3];
            EXPECT_EQ(extended_payload_len, websocket_payload_size);
            tl_ptr += 4;
        }
#endif

        uint32_t header = (tl_ptr[0]) | (tl_ptr[1] << 8) | (tl_ptr[2] << 16) | (tl_ptr[3] << 24);
        
        EXPECT_EQ(header & 0x000fffff, signal_no);
        EXPECT_EQ((header & 0x30000000) >> 28, type);
        
        if (payload_size <= 255) {
            EXPECT_EQ((header & 0x0ff00000) >> 20, payload_size);
        } else {
            EXPECT_EQ((header & 0x0ff00000) >> 20, 0);
            uint32_t extended_size = (tl_ptr[4]) | (tl_ptr[5] << 8) | (tl_ptr[6] << 16) | (tl_ptr[7] << 24);
            EXPECT_EQ(extended_size, payload_size);
        }
    }
};

TEST_F(StreamingPacketTest, BuildPacketMetaStream) {
    tl_packet_t packet;
    char meta_data[] = "some meta data";
    uint32_t meta_size = sizeof(meta_data);
    
    build_packet_meta_stream(&packet, meta_data, meta_size);
    
    EXPECT_EQ(packet.packet_type, TYPE_META);
    EXPECT_EQ(packet.signal_number, 0);
    EXPECT_EQ(packet.payload.meta.meta_data, meta_data);
    EXPECT_EQ(packet.payload_size, meta_size + 4);
}

TEST_F(StreamingPacketTest, BuildPacketMetaSignal) {
    tl_packet_t packet;
    char meta_data[] = "signal meta data";
    uint32_t meta_size = sizeof(meta_data);
    uint32_t signal_no = 42;
    
    build_packet_meta_signal(&packet, meta_data, meta_size, signal_no);
    
    EXPECT_EQ(packet.packet_type, TYPE_META);
    EXPECT_EQ(packet.signal_number, signal_no);
    EXPECT_EQ(packet.payload.meta.meta_data, meta_data);
    EXPECT_EQ(packet.payload_size, meta_size + 4);
}

TEST_F(StreamingPacketTest, SendPacketMeta) {
    struct stream s;
    s.stream = mock_stream_send;
    
    tl_packet_t packet;
    char meta_data[] = { 0x01, 0x02, 0x03 };
    uint32_t meta_size = 3;
    
    build_packet_meta_stream(&packet, meta_data, meta_size);
    
    int result = openDAQ_streaming_send_packet(&s, &packet);
    
    EXPECT_GT(result, 0);
    EXPECT_EQ(last_sent_bytes, result);
    
    VerifyHeader((uint8_t*)last_sent_buffer, 0, TYPE_META, meta_size + 4);
    
    // Check meta payload
    uint32_t tl_header_size = (meta_size + 4 > 255) ? 8 : 4;
#ifdef WEBSOCKET_STREAMING
    uint32_t ws_header_size = (tl_header_size + meta_size + 4 < 126) ? 2 : 4;
    const char* payload_ptr = last_sent_buffer + ws_header_size + tl_header_size;
#else
    const char* payload_ptr = last_sent_buffer + tl_header_size;
#endif

    uint32_t meta_type;
    memcpy(&meta_type, payload_ptr, 4);
    EXPECT_EQ(meta_type, 2); // METAINFORMATION_MSGPACK
    EXPECT_EQ(memcmp(payload_ptr + 4, meta_data, meta_size), 0);
}

TEST_F(StreamingPacketTest, SerializeDifferentDataTypes) {
    struct TestCase {
        signal_data_type_e type;
        std::vector<uint8_t> input;
        std::vector<uint8_t> expected_le;
    };

    std::vector<TestCase> cases = {
        {signal_type_uint8, {0xDE}, {0xDE}},
        {signal_type_int16, {0x34, 0x12}, {0x34, 0x12}}, // Assumes host is little endian for input or we just provide bytes
        {signal_type_uint32, {0x78, 0x56, 0x34, 0x12}, {0x78, 0x56, 0x34, 0x12}},
        {signal_type_uint64, {0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12}, {0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12}}
    };

    for (auto& tc : cases) {
        signal_definition_t def = {0};
        def.rule = signal_explicit_rule;
        def.datatype = tc.type;
        
        signal_t *signal = signals_add_signal(&def, NULL);
        ASSERT_NE(signal, nullptr);
        
        uint8_t buffer[100];
        int result = openDAQ_streaming_serialize_explicit_signal(buffer, sizeof(buffer), signal, tc.input.data(), 1);
        
        EXPECT_GT(result, 4);
        VerifyHeader(buffer, signal_get_signal_no(signal), TYPE_DATA, tc.expected_le.size());

        uint32_t tl_header_size = (tc.expected_le.size() > 255) ? 8 : 4;
#ifdef WEBSOCKET_STREAMING
        uint32_t ws_header_size = (tl_header_size + tc.expected_le.size() < 126) ? 2 : 4;
        const uint8_t* payload_ptr = buffer + ws_header_size + tl_header_size;
#else
        const uint8_t* payload_ptr = buffer + tl_header_size;
#endif
        EXPECT_EQ(memcmp(payload_ptr, tc.expected_le.data(), tc.expected_le.size()), 0);
    }
}

TEST_F(StreamingPacketTest, SerializeImplicitSignal) {
    signal_definition_t def = {0};
    def.rule = signal_constant_rule; // or linear
    def.datatype = signal_type_uint32;
    
    signal_t *signal = signals_add_signal(&def, NULL);
    ASSERT_NE(signal, nullptr);
    
    uint32_t data = 0x12345678;
    uint64_t index = 0x1122334455667788ULL;
    uint8_t buffer[100];
    
    int result = openDAQ_streaming_serialize_implicit_signal(buffer, sizeof(buffer), index, signal, &data);
    
    EXPECT_GT(result, 4);
    uint32_t expected_payload_size = 8 + 4; // index(8) + data(4)
    VerifyHeader(buffer, signal_get_signal_no(signal), TYPE_DATA, expected_payload_size);
    
    uint32_t tl_header_size = (expected_payload_size > 255) ? 8 : 4;
#ifdef WEBSOCKET_STREAMING
    uint32_t ws_header_size = (tl_header_size + expected_payload_size < 126) ? 2 : 4;
    const uint8_t* payload_ptr = buffer + ws_header_size + tl_header_size;
#else
    const uint8_t* payload_ptr = buffer + tl_header_size;
#endif

    // Check index (little endian)
    uint64_t serialized_index;
    memcpy(&serialized_index, payload_ptr, 8);
    EXPECT_EQ(serialized_index, index);
    
    // Check data (little endian)
    uint32_t serialized_data;
    memcpy(&serialized_data, payload_ptr + 8, 4);
    EXPECT_EQ(serialized_data, data);
}

TEST_F(StreamingPacketTest, LargePayloadHeader) {
    signal_definition_t def = {0};
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_uint8;
    
    signal_t *signal = signals_add_signal(&def, NULL);
    ASSERT_NE(signal, nullptr);
    
    std::vector<uint8_t> data(300, 0xAA);
    std::vector<uint8_t> buffer(400);
    
    int result = openDAQ_streaming_serialize_explicit_signal(buffer.data(), buffer.size(), signal, data.data(), data.size());
    
    EXPECT_GT(result, 8);
    VerifyHeader(buffer.data(), signal_get_signal_no(signal), TYPE_DATA, data.size());

    uint32_t tl_header_size = (data.size() > 255) ? 8 : 4;
#ifdef WEBSOCKET_STREAMING
    uint32_t ws_header_size = (tl_header_size + data.size() < 126) ? 2 : 4;
    const uint8_t* payload_ptr = buffer.data() + ws_header_size + tl_header_size;
#else
    const uint8_t* payload_ptr = buffer.data() + tl_header_size;
#endif
    EXPECT_EQ(memcmp(payload_ptr, data.data(), data.size()), 0);
}

TEST_F(StreamingPacketTest, BufferTooSmall) {
    signal_definition_t def = {0};
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_uint32;
    
    signal_t *signal = signals_add_signal(&def, NULL);
    
    uint32_t data = 0x12345678;
    uint8_t buffer[4]; // only enough for partial header
    
    int result = openDAQ_streaming_serialize_explicit_signal(buffer, sizeof(buffer), signal, &data, 1);
    
    EXPECT_EQ(result, -1);
}

TEST_F(StreamingPacketTest, WebSocketHeaderBoundaries) {
    signal_definition_t def = {0};
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_uint8;
    signal_t *signal = signals_add_signal(&def, NULL);

    // Case 1: Payload size results in websocket_payload_size < 126
    // TL header (4) + Payload (121) = 125
    {
        std::vector<uint8_t> data(121, 0x00);
        std::vector<uint8_t> buffer(256);
        int result = openDAQ_streaming_serialize_explicit_signal(buffer.data(), buffer.size(), signal, data.data(), data.size());
        EXPECT_GT(result, 0);
        VerifyHeader(buffer.data(), signal_get_signal_no(signal), TYPE_DATA, data.size());
        
#ifdef WEBSOCKET_STREAMING
        EXPECT_EQ(buffer[1], 125); // 7-bit length
#endif
    }

    // Case 2: Payload size results in websocket_payload_size >= 126
    // TL header (4) + Payload (122) = 126
    {
        std::vector<uint8_t> data(122, 0x00);
        std::vector<uint8_t> buffer(256);
        int result = openDAQ_streaming_serialize_explicit_signal(buffer.data(), buffer.size(), signal, data.data(), data.size());
        EXPECT_GT(result, 0);
        VerifyHeader(buffer.data(), signal_get_signal_no(signal), TYPE_DATA, data.size());

#ifdef WEBSOCKET_STREAMING
        EXPECT_EQ(buffer[1], 126); // Extended 16-bit length sentinel
        EXPECT_EQ(buffer[2], 0);   // High byte of 126
        EXPECT_EQ(buffer[3], 126); // Low byte of 126
#endif
    }
}

TEST_F(StreamingPacketTest, SerializeComplexTypes) {
    // Complex32 is 2x real32
    signal_definition_t def = {0};
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_complex32;
    
    signal_t *signal = signals_add_signal(&def, NULL);
    
    float data[2] = {1.0f, 2.0f};
    uint8_t buffer[100];
    
    int result = openDAQ_streaming_serialize_explicit_signal(buffer, sizeof(buffer), signal, data, 1);
    
    EXPECT_GT(result, 4);
    VerifyHeader(buffer, signal_get_signal_no(signal), TYPE_DATA, 8);
    
    uint32_t tl_header_size = (8 > 255) ? 8 : 4;
#ifdef WEBSOCKET_STREAMING
    uint32_t ws_header_size = (tl_header_size + 8 < 126) ? 2 : 4;
    const uint8_t* payload_ptr = buffer + ws_header_size + tl_header_size;
#else
    const uint8_t* payload_ptr = buffer + tl_header_size;
#endif

    float serialized_real, serialized_imag;
    memcpy(&serialized_real, payload_ptr, 4);
    memcpy(&serialized_imag, payload_ptr + 4, 4);
    EXPECT_FLOAT_EQ(serialized_real, 1.0f);
    EXPECT_FLOAT_EQ(serialized_imag, 2.0f);
}
