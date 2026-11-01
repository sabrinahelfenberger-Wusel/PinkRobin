#include "pinkrobin/weather_behavior.hpp"

namespace pinkrobin {

    Transformation WeatherBehavior::propose(
        const WorldModel& world) const noexcept {

        const auto weather = world.weather();

        if (!weather) {
            return Transformation::none;
        }

        const auto& forecast = weather->forecast;

        if (forecast.precipitation_mm >= 10.0 &&
            forecast.wind_gust_kmh >= 60.0) {
            return Transformation::octopus;
        }

        if (forecast.precipitation_mm >= 0.5) {
            return Transformation::raincoat;
        }

        return Transformation::none;
    }
}