#include <gtest/gtest.h>
#include "streaming_meta.h"
#include "mpack.h"
#include <cstring>

class StreamingMetaTest : public ::testing::Test {
protected:
    char buffer[1024];
    
    void SetUp() override {
        memset(buffer, 0, sizeof(buffer));
    }

    void ExpectNodeStringEq(mpack_node_t node, const char* expected) {
        ASSERT_EQ(mpack_node_type(node), mpack_type_str);
        size_t len = mpack_node_strlen(node);
        EXPECT_EQ(len, strlen(expected));
        EXPECT_EQ(strncmp(mpack_node_str(node), expected, len), 0);
    }
};

TEST_F(StreamingMetaTest, BuildStreamVersion) {
    int len = build_mpack_meta_stream_version(buffer, sizeof(buffer));
    ASSERT_GT(len, 0);
    
    // Verify using mpack reader
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, buffer, len);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);
    
    mpack_node_t method = mpack_node_map_cstr(root, "method");
    ExpectNodeStringEq(method, "apiVersion");
    
    mpack_node_t params = mpack_node_map_cstr(root, "params");
    mpack_node_t version = mpack_node_map_cstr(params, "version");
    ExpectNodeStringEq(version, "1.0.1");
    
    mpack_tree_destroy(&tree);
}

TEST_F(StreamingMetaTest, BuildStreamInit) {
    const char* stream_id = "test-stream";
    int len = build_mpack_meta_stream_init(buffer, sizeof(buffer), stream_id);
    ASSERT_GT(len, 0);
    
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, buffer, len);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);
    
    mpack_node_t method = mpack_node_map_cstr(root, "method");
    ExpectNodeStringEq(method, "init");
    
    mpack_node_t params = mpack_node_map_cstr(root, "params");
    mpack_node_t sid = mpack_node_map_cstr(params, "streamId");
    ExpectNodeStringEq(sid, stream_id);
    
    mpack_tree_destroy(&tree);
}

TEST_F(StreamingMetaTest, BuildSignalSubscribed) {
    const char* signal_id = "sig1";
    int len = build_mpack_meta_signal_subscribed(buffer, sizeof(buffer), signal_id);
    ASSERT_GT(len, 0);
    
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, buffer, len);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);
    
    ExpectNodeStringEq(mpack_node_map_cstr(root, "method"), "subscribe");
    
    mpack_node_t params = mpack_node_map_cstr(root, "params");
    ExpectNodeStringEq(mpack_node_map_cstr(params, "signalId"), signal_id);
    
    mpack_tree_destroy(&tree);
}
