#pragma once

#include <cstdint>
#include <variant>

namespace pinkrobin {

    enum class WeatherAnswerKind : std::uint32_t {
        unknown = 0,
        dry = 1,
        rain = 2,
        storm = 3
    };

    struct WeatherAnswer {
        WeatherAnswerKind kind;
    };

    using OutputPayload = std::variant<WeatherAnswer>;

    struct Output {
        std::uint64_t sequence;
        OutputPayload payload;
    };

}