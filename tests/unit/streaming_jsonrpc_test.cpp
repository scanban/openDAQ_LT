#include <gtest/gtest.h>
#include "streaming_jsonrpc.h"
#include "streaming_signals.h"
#include "streaming_handler.h"
#include <cstring>

extern char mock_webs_send_buffer[2048];
extern int mock_webs_send_len;
extern char mock_webs_copy_buffer[2048];
extern int mock_webs_copy_len;

static uint64_t mock_on_subscribe(const struct stream *stream, signal_t *signal) {
    (void)stream;
    (void)signal;
    return 123;
}

static void mock_on_unsubscribe(const struct stream *stream, signal_t *signal) {
    (void)stream;
    (void)signal;
}

class StreamingJsonRpcTest : public ::testing::Test {
protected:
    struct streaming_callbacks callbacks;

    void SetUp() override {
        callbacks.on_subscribe = mock_on_subscribe;
        callbacks.on_unsubscribe = mock_on_unsubscribe;
        streaming_cbs = &callbacks;

        signals_init();
        memset(mock_webs_send_buffer, 0, sizeof(mock_webs_send_buffer));
        mock_webs_send_len = 0;
        memset(mock_webs_copy_buffer, 0, sizeof(mock_webs_copy_buffer));
        mock_webs_copy_len = 0;
        
        char stream_id[9] = "stream0";
        streaming_jsonrpc_init(stream_id);
    }
};

TEST_F(StreamingJsonRpcTest, SubscribeSuccess) {
    // Add a signal to subscribe to
    signal_definition_t def = {0};
    def.name = "sig1";
    def.signaltype = signal_type_value;
    signals_add_signal(&def, NULL);

    const char* req = "{\"jsonrpc\": \"2.0\", \"method\": \"stream0.subscribe\", \"params\": [\"sig1\"], \"id\": 1}";
    strcpy(mock_webs_copy_buffer, req);
    mock_webs_copy_len = strlen(req);
    
    streaming_jsonrpc_callback(NULL, NULL, "POST", NULL, "application/json", "/streaming_jsonrpc", mock_webs_copy_len);
    
    EXPECT_GT(mock_webs_send_len, 0);
    EXPECT_TRUE(strstr(mock_webs_send_buffer, "\"result\":true") != NULL);
}

TEST_F(StreamingJsonRpcTest, SubscribeNotFound) {
    const char* req = "{\"jsonrpc\": \"2.0\", \"method\": \"stream0.subscribe\", \"params\": [\"nonexistent\"], \"id\": 1}";
    strcpy(mock_webs_copy_buffer, req);
    mock_webs_copy_len = strlen(req);
    
    streaming_jsonrpc_callback(NULL, NULL, "POST", NULL, "application/json", "/streaming_jsonrpc", mock_webs_copy_len);
    
    EXPECT_GT(mock_webs_send_len, 0);
    EXPECT_TRUE(strstr(mock_webs_send_buffer, "\"error\"") != NULL);
}
