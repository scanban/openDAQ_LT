#include <gtest/gtest.h>
#include "stream_id.h"
#include <string>

class StreamIdTest : public ::testing::Test {
protected:
    void SetUp() override {
        streaming_streams_init();
    }
};

TEST_F(StreamIdTest, StreamMallocSetsSocketAndId) {
    int test_socket = 42;
    const char* test_id = "test_stream_id";
    
    struct stream* s = stream_malloc(test_socket, test_id);
    
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->socket_handle, test_socket);
    EXPECT_STREQ(s->id, test_id);
}

TEST_F(StreamIdTest, StreamFreeClearsSocketHandle) {
    int test_socket = 42;
    struct stream* s = stream_malloc(test_socket, "id");
    
    stream_free(s);
    
    EXPECT_EQ(s->socket_handle, 0);
}
