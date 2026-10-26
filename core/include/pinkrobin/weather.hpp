#pragma once
#include <cstdint>

namespace pinkrobin {

    struct WeatherForecast {
        double temperature_c;
        double precipitation_mm;
        double wind_speed_kmh;
        double wind_gust_kmh;

        std::uint64_t valid_from_ms;
        std::uint64_t valid_until_ms;
    };

}