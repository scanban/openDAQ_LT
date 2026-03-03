#include <gtest/gtest.h>
#include "streaming_signals.h"
#include "streaming_handler.h"
#include <cstring>

static uint64_t mock_on_subscribe(const struct stream *stream, signal_t *signal) {
    (void)stream;
    (void)signal;
    return 123; // mock valueIndex
}

static void mock_on_unsubscribe(const struct stream *stream, signal_t *signal) {
    (void)stream;
    (void)signal;
}

class StreamingSignalsTest : public ::testing::Test {
protected:
    struct streaming_callbacks callbacks;

    void SetUp() override {
        callbacks.on_subscribe = mock_on_subscribe;
        callbacks.on_unsubscribe = mock_on_unsubscribe;
        streaming_cbs = &callbacks;
        signals_init();
    }
};

TEST_F(StreamingSignalsTest, AddTableAndSignals) {
    signal_definition_t defs[2] = {
        {
            .name = "sig1", .rule = signal_explicit_rule, .datatype = signal_type_real64, .signaltype = signal_type_value,
            .hidden = false, .delta = 0, .time = nullptr, .range = nullptr, .postScaling = nullptr
        },
        {
            .name = "sig2", .rule = signal_explicit_rule, .datatype = signal_type_real64, .signaltype = signal_type_time,
            .hidden = false, .delta = 0, .time = nullptr, .range = nullptr, .postScaling = nullptr
        }
    };
    
    signal_table_t* table = signals_add_table(defs, 2, "table1");
    ASSERT_NE(table, nullptr);
    EXPECT_STREQ(table->tableId, "table1");
    EXPECT_EQ(table->signal_counter, 2);
}

TEST_F(StreamingSignalsTest, SubscribeSignal) {
    signal_definition_t def = {
        .name = "sig1",
        .rule = signal_explicit_rule,
        .datatype = signal_type_real64,
        .signaltype = signal_type_value,
        .hidden = false,
        .delta = 0,
        .time = nullptr,
        .range = nullptr,
        .postScaling = nullptr
    };
    signal_table_t* table = signals_add_table(&def, 1, "table1");
    ASSERT_NE(table, nullptr);

    struct stream s;
    s.socket_handle = 1;

    int ret = signals_subscribe(&s, "sig1");
    EXPECT_EQ(ret, 0);
    
    // Check if signal is subscribed
    // We need access to signals array or a way to check.
    // signal_has_subscription can be used.
    EXPECT_TRUE(signal_has_subscription(table->signals));
}
