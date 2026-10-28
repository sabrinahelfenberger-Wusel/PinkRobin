#include "pinkrobin/world_model.hpp"

namespace pinkrobin {

    std::optional<WeatherObservation>
        WorldModel::weather() const noexcept {
        return weather_;
    }

    void WorldModel::update_weather(
        const WeatherObservation& observation) noexcept {

        weather_ = observation;
    }

    void WorldModel::expire_weather(
        std::uint64_t now_ms) noexcept {

        if (weather_ &&
            weather_->forecast.valid_until_ms <= now_ms) {
            weather_.reset();
        }
    }
}