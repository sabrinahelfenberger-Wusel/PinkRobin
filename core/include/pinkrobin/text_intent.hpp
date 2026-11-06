#pragma once

#include <array>
#include <cstdint>

#include "pinkrobin/conversation_state.hpp"
#include "pinkrobin/text_input.hpp"

namespace pinkrobin {

    enum class TextIntentKind : std::uint32_t {
        unknown = 0,
        weather_question = 1,
        swiss_postal_code = 2
    };

    struct TextIntent {
        TextIntentKind kind{ TextIntentKind::unknown };
        std::array<char, 4> postal_code{};
    };

    TextIntent recognize_text(
        const TextReceived& input,
        ExpectedReply expected_reply) noexcept;

}