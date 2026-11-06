#include "pinkrobin/conversation_state.hpp"
#include "test_support.hpp"

using namespace pinkrobin;

namespace {

    void test_empty_context() {
        ConversationState state;

        CHECK(!state.expire(120000));
        CHECK(state.topic == ConversationTopic::none);
        CHECK(state.expected_reply == ExpectedReply::none);
        CHECK(!state.last_input_at_ms.has_value());
    }

    void test_expiry_boundary() {
        ConversationState state{
            ConversationTopic::weather,
            ExpectedReply::swiss_postal_code,
            8000
        };

        CHECK(!state.expire(127999));
        CHECK(state.expected_reply ==
            ExpectedReply::swiss_postal_code);
        CHECK(state.last_input_at_ms == 8000);

        CHECK(state.expire(128000));
        CHECK(state.topic == ConversationTopic::none);
        CHECK(state.expected_reply == ExpectedReply::none);
        CHECK(!state.last_input_at_ms.has_value());

        CHECK(!state.expire(128001));
    }

    void test_reversed_time() {
        ConversationState state{
            ConversationTopic::weather,
            ExpectedReply::swiss_postal_code,
            8000
        };

        CHECK(!state.expire(7999));
        CHECK(state.topic == ConversationTopic::weather);
        CHECK(state.expected_reply ==
            ExpectedReply::swiss_postal_code);
        CHECK(state.last_input_at_ms == 8000);
    }

}

int main() {
    return test_support::run_tests({
        { "conversation: empty_context", test_empty_context },
        { "conversation: expiry_boundary", test_expiry_boundary },
        { "conversation: reversed_time", test_reversed_time },
        });
}