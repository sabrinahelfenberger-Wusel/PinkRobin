#include "pinkrobin/core.hpp"
#include <cmath>

namespace pinkrobin {
    Result Core::validate_weather(
        const WeatherForecast& forecast,
        std::uint64_t now_ms) noexcept {

        if (!std::isfinite(forecast.temperature_c) ||
            !std::isfinite(forecast.precipitation_mm) ||
            !std::isfinite(forecast.wind_speed_kmh) ||
            !std::isfinite(forecast.wind_gust_kmh)) {
            return Result::invalid_argument;
        }

        if (forecast.precipitation_mm < 0 ||
            forecast.wind_speed_kmh < 0 ||
            forecast.wind_gust_kmh < 0 ||
            forecast.valid_from_ms >= forecast.valid_until_ms ||
            forecast.valid_until_ms <= now_ms) {
            return Result::invalid_argument;
        }

        return Result::ok;
    }

    Result Core::post_weather(
        const WeatherUpdated& message) noexcept {

        const auto result =
            validate_weather(message.forecast, now_);

        if (result != Result::ok) {
            return result;
        }

        pending_weather_ = message;
        pending_weather_source_ = InputSource::backend;
        pending_weather_received_at_ms_ = now_;

        return Result::ok;
    }

    std::optional<WeatherForecast> Core::weather() const noexcept {
        const auto observation = world_.weather();

        if (!observation) {
            return std::nullopt;
        }

        return observation->forecast;
    }

    Transformation Core::choose_weather_transformation() const noexcept {
        return weather_behavior_.propose(world_);
    }

    void Core::process_weather_reaction() noexcept {
        const auto proposed = choose_weather_transformation();

        if (proposed == Transformation::none) {
            return;
        }

        if (!behavior_controller_.can_start_weather_reaction(
            state_,
            now_)) {
            return;
        }

        state_.transformation = proposed;
        transformation_started_ = now_;

        behavior_controller_.mark_weather_reaction_started(now_);

        ++revision_;
    }

}