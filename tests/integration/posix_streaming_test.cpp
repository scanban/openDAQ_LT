#include <gtest/gtest.h>
#include "streaming_handler.h"
#include "streaming_transport.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mpack.h"

#if STREAMING_TRANSPORT == STREAMING_TRANSPORT_POSIX

static const struct stream* last_stream = NULL;
static void on_connect_cb(const struct stream *stream) {
    last_stream = stream;
}

static void* server_thread(void* arg) {
    (void)arg;
    streaming_start();
    return NULL;
}

class PosixStreamingIntegrationTest : public ::testing::Test {
protected:
    pthread_t thread;
    static struct streaming_callbacks callbacks;

    void SetUp() override {
        last_stream = NULL;
        callbacks.on_connect = on_connect_cb;
        callbacks.on_subscribe = NULL;
        callbacks.on_unsubscribe = NULL;
        streaming_init(&callbacks);
        pthread_create(&thread, NULL, server_thread, NULL);
        // Give the server a moment to start
        usleep(100000);
    }

    void TearDown() override {
        streaming_stop();
        pthread_join(thread, NULL);
    }
};

struct streaming_callbacks PosixStreamingIntegrationTest::callbacks;

TEST_F(PosixStreamingIntegrationTest, TCPConnectionLifecycle) {
#if STREAMING_PROTOCOL == STREAMING_PROTOCOL_TCP
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(client_sock, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(STREAMING_TCP_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to server
    int ret = connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    ASSERT_EQ(ret, 0);

    // Read meta stream version (should be received immediately)
    char buffer[256];
    ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
    EXPECT_GT(bytes_received, 0);

    close(client_sock);
#endif
}

TEST_F(PosixStreamingIntegrationTest, UNIXConnectionLifecycle) {
#if STREAMING_PROTOCOL == STREAMING_PROTOCOL_UNIX
    int client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_GE(client_sock, 0);

    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, STREAMING_UNIX_PATH, sizeof(serv_addr.sun_path) - 1);

    // Connect to server
    int ret = connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    ASSERT_EQ(ret, 0);

    // Read meta stream version
    char buffer[256];
    ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
    EXPECT_GT(bytes_received, 0);

    close(client_sock);
#endif
}

TEST_F(PosixStreamingIntegrationTest, InitialSignals) {
    // Add a dummy signal via table
    static signal_definition_t def;
    memset(&def, 0, sizeof(def));
    def.name = "test_signal";
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_real64;
    def.signaltype = signal_type_value;
    def.hidden = false;
    signals_add_table(&def, 1, "test_table");

    int client_sock;
#if STREAMING_PROTOCOL == STREAMING_PROTOCOL_TCP
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr_in;
    serv_addr_in.sin_family = AF_INET;
    serv_addr_in.sin_port = htons(STREAMING_TCP_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr_in.sin_addr);
    connect(client_sock, (struct sockaddr *)&serv_addr_in, sizeof(serv_addr_in));
#else
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un serv_addr_un;
    memset(&serv_addr_un, 0, sizeof(serv_addr_un));
    serv_addr_un.sun_family = AF_UNIX;
    strncpy(serv_addr_un.sun_path, STREAMING_UNIX_PATH, sizeof(serv_addr_un.sun_path) - 1);
    connect(client_sock, (struct sockaddr *)&serv_addr_un, sizeof(serv_addr_un));
#endif

    // Server should send: meta stream version, meta stream init, available signals
    char buffer[1024];
    ssize_t total_received = 0;
    
    // Give it a moment to send all
    usleep(150000);
    total_received = recv(client_sock, buffer, sizeof(buffer), MSG_DONTWAIT);
    
    ASSERT_GT(total_received, 40);

    // Parse the concatenated packets
    const uint8_t* ptr = (const uint8_t*)buffer;
    const uint8_t* end = ptr + total_received;
    bool signal_found = false;

    while (ptr < end) {
        if (end - ptr < 4) break;
        uint32_t header = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
        uint32_t packet_type = (header >> 28) & 0x3;
        uint32_t payload_size = (header >> 20) & 0xFF;
        ptr += 4;

        if (payload_size == 0) {
            if (end - ptr < 4) break;
            payload_size = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
            ptr += 4;
        }

        if (packet_type == 2) { // TYPE_META
            // First 4 bytes of meta payload is meta_type (METAINFORMATION_MSGPACK = 2)
            if (end - ptr < 4) break;
            ptr += 4;
            uint32_t mpack_size = payload_size - 4;
            if (end - ptr < mpack_size) break;

            mpack_reader_t reader;
            mpack_reader_init_data(&reader, (const char*)ptr, mpack_size);
            
            mpack_tag_t tag = mpack_read_tag(&reader);
            if (mpack_tag_type(&tag) == mpack_type_map) {
                uint32_t map_size = mpack_tag_map_count(&tag);
                for (uint32_t i = 0; i < map_size; i++) {
                    char key[32];
                    mpack_expect_cstr(&reader, key, sizeof(key));
                    if (strcmp(key, "method") == 0) {
                        char method[32];
                        mpack_expect_cstr(&reader, method, sizeof(method));
                        if (strcmp(method, "available") != 0) {
                          break;
                        }
                    } else if (strcmp(key, "params") == 0) {
                        uint32_t params_size = mpack_expect_map(&reader);
                        for (uint32_t k = 0; k < params_size; k++) {
                            char pkey[32];
                            mpack_expect_cstr(&reader, pkey, sizeof(pkey));
                            if (strcmp(pkey, "signalIds") == 0) {
                                uint32_t array_size = mpack_expect_array(&reader);
                                for (uint32_t j = 0; j < array_size; j++) {
                                    char sig_id[64];
                                    mpack_expect_cstr(&reader, sig_id, sizeof(sig_id));
                                    if (strcmp(sig_id, "test_signal") == 0) {
                                        signal_found = true;
                                    }
                                }
                            } else {
                                mpack_discard(&reader);
                            }
                        }
                    } else {
                        mpack_discard(&reader);
                    }
                }
            }
            mpack_reader_destroy(&reader);
            ptr += mpack_size;
        } else {
            ptr += payload_size;
        }
    }

    EXPECT_TRUE(signal_found);

    close(client_sock);
}

TEST_F(PosixStreamingIntegrationTest, ClientDisconnection) {
    // Add a signal via table
    static signal_definition_t def;
    memset(&def, 0, sizeof(def));
    def.name = "disc_signal";
    def.rule = signal_explicit_rule;
    def.datatype = signal_type_real64;
    def.signaltype = signal_type_value;
    def.hidden = false;
    signal_table_t* table = signals_add_table(&def, 1, "disc_table");
    signal_t* sig = table->signals;

    int client_sock;
#if STREAMING_PROTOCOL == STREAMING_PROTOCOL_TCP
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr_in;
    serv_addr_in.sin_family = AF_INET;
    serv_addr_in.sin_port = htons(STREAMING_TCP_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr_in.sin_addr);
    connect(client_sock, (struct sockaddr *)&serv_addr_in, sizeof(serv_addr_in));
#else
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un serv_addr_un;
    memset(&serv_addr_un, 0, sizeof(serv_addr_un));
    serv_addr_un.sun_family = AF_UNIX;
    strncpy(serv_addr_un.sun_path, STREAMING_UNIX_PATH, sizeof(serv_addr_un.sun_path) - 1);
    connect(client_sock, (struct sockaddr *)&serv_addr_un, sizeof(serv_addr_un));
#endif

    // Wait for server to process connection
    usleep(50000);
    ASSERT_NE(last_stream, nullptr);
    
    // Subscribe the signal to this stream
    signals_subscribe(last_stream, "disc_signal");
    EXPECT_TRUE(signal_has_subscription(sig));

    // Disconnect client
    close(client_sock);

    // Give server time to detect disconnection (it checks every 10ms)
    usleep(100000);

    // Verify signal is no longer subscribed
    EXPECT_FALSE(signal_has_subscription(sig));
}

#endif
