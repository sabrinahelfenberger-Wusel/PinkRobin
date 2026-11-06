#pragma once

#include <cstdint>
#include <optional>

namespace pinkrobin {

    enum class ConversationTopic : std::uint32_t {
        none = 0,
        weather = 1
    };

    enum class ExpectedReply : std::uint32_t {
        none = 0,
        swiss_postal_code = 1
    };

    struct ConversationState {
        ConversationTopic topic{ ConversationTopic::none };
        ExpectedReply expected_reply{ ExpectedReply::none };

        std::optional<std::uint64_t> last_input_at_ms;

        bool expire(std::uint64_t now_ms) noexcept {
            if (!last_input_at_ms ||
                now_ms < *last_input_at_ms ||
                now_ms - *last_input_at_ms < 120000) {
                return false;
            }

            topic = ConversationTopic::none;
            expected_reply = ExpectedReply::none;
            last_input_at_ms.reset();

            return true;
        }
    };

}