#pragma once
#include <cstdint>
#include <array>

namespace pinkrobin {

    enum class WeatherLocationKind : std::uint32_t {
        unspecified = 0,
        default_location = 1,
        selected_location = 2
    };

    struct WeatherLocation {
        WeatherLocationKind kind{
            WeatherLocationKind::unspecified
        };

        std::array<char, 4> postal_code{};
    };

    inline bool is_valid_weather_location(
        const WeatherLocation& location) noexcept {

        switch (location.kind) {
        case WeatherLocationKind::unspecified:
            for (const auto value : location.postal_code) {
                if (value != '\0') {
                    return false;
                }
            }
            return true;

        case WeatherLocationKind::default_location:
        case WeatherLocationKind::selected_location:
            for (const auto digit : location.postal_code) {
                if (digit < '0' || digit > '9') {
                    return false;
                }
            }
            return true;
        }

        return false;
    }

    struct WeatherForecast {
        double temperature_c;
        double precipitation_mm;
        double wind_speed_kmh;
        double wind_gust_kmh;

        std::uint64_t valid_from_ms;
        std::uint64_t valid_until_ms;
    };

}