#pragma once

#include <cstdint>
#include <variant>
#include <array>

namespace pinkrobin {

    enum class WeatherAnswerKind : std::uint32_t {
        unknown = 0,
        dry = 1,
        rain = 2,
        storm = 3
    };

    enum class ConversationReplyKind : std::uint32_t {
        unknown_input = 0,
        ask_postal_code = 1,
        invalid_postal_code = 2,
        weather_unavailable = 3,
        attention = 4,
        greeting = 5,
        gratitude = 6
    };

    struct ConversationReply {
        ConversationReplyKind kind;
    };

    struct WeatherFetchRequested {
        std::uint64_t request_id;
        std::array<char, 4> postal_code;
    };

    struct WeatherAnswer {
        WeatherAnswerKind kind;
    };

    using OutputPayload = std::variant<
        WeatherAnswer,
        ConversationReply,
        WeatherFetchRequested
    >;

    // Länge in UTF-8-Bytes, ohne abschließendes Nullzeichen.
    struct OutputText {
        static constexpr std::uint32_t capacity = 512;

        std::array<char, capacity> utf8{};
        std::uint32_t length{ 0 };
    };

    struct Output {
        std::uint64_t sequence;
        OutputPayload payload;
        OutputText text{};
    };
}