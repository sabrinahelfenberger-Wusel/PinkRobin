#pragma once

#include <cstdint>
#include <optional>
#include "pinkrobin/message.hpp"

namespace pinkrobin {

    struct WeatherObservation {
        WeatherForecast forecast;
        InputSource source;
        std::uint64_t received_at_ms;
    };

    class WorldModel final {
    public:
        std::optional<WeatherObservation> weather() const noexcept;

        void update_weather(const WeatherObservation& observation) noexcept;
        void expire_weather(std::uint64_t now_ms) noexcept;

    private:
        std::optional<WeatherObservation> weather_;
    };
}