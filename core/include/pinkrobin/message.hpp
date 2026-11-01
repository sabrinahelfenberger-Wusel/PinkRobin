#pragma once

#include <cstdint>
#include <variant>
#include "pinkrobin/weather.hpp"

namespace pinkrobin {

    enum class Event : std::uint32_t {
        restart = 0, 
        heart_touched = 1, 
        head_started = 2, 
        head_ended = 3, 
        interaction_cancelled = 4 
    };

    enum class InputSource : std::uint32_t {
        web = 0,
        companion = 1,
        device = 2,
        backend = 3
    };

    enum class MessageAcceptance : std::uint32_t {
        accepted = 0,
        replaced = 1,
        invalid_argument = 2,
        time_reversed = 3,
        full = 4
    };

    struct MessageMetadata {
        InputSource source;
    };

    struct InteractionReceived {
        Event event;
    };

    struct WeatherUpdated {
        WeatherForecast forecast;
    };

    using MessagePayload = std::variant<
        InteractionReceived,
        WeatherUpdated
    >;
    
    struct Message {
        MessageMetadata metadata;
        MessagePayload payload;
    };

}