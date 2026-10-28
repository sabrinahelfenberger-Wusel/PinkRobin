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
        pending_weather_received_at_ms_ = now_;

        return Result::ok;
    }

    std::optional<WeatherForecast> Core::weather() const noexcept {
        return weather_;
    }

    Transformation Core::choose_weather_transformation() const noexcept {
        if (!weather_) {
            return Transformation::none;
        }

        if (weather_->precipitation_mm >= 10.0 &&
            weather_->wind_gust_kmh >= 60.0) {
            return Transformation::octopus;
        }

        if (weather_->precipitation_mm >= 0.5) {
            return Transformation::raincoat;
        }

        return Transformation::none;
    }

    void Core::process_weather_reaction() noexcept {
        const auto proposed = choose_weather_transformation();

        // Kein passendes Wetter mehr: Vormerkung verwerfen.
        if (proposed == Transformation::none) {
            pending_transformation_ = Transformation::none;
            weather_retry_started_.reset();
            return;
        }

        // Ein neuer Vorschlag darf sofort geprüft werden.
        if (pending_transformation_ != proposed) {
            pending_transformation_ = proposed;
            weather_retry_started_.reset();
        }

        // Nach einem tatsächlichen Start: 30 Minuten Pause.
        if (weather_reaction_started_ &&
            now_ - *weather_reaction_started_ < 1800000) {
            return;
        }

        // Nach einer Ablehnung: zehn Sekunden warten.
        if (weather_retry_started_ &&
            now_ - *weather_retry_started_ < 10000) {
            return;
        }

        if (!can_start_transformation()) {
            weather_retry_started_ = now_;
            return;
        }

        // Robin ist bereit: Verwandlung tatsächlich starten.
        transformation_ = pending_transformation_;
        transformation_started_ = now_;
        weather_reaction_started_ = now_;

        pending_transformation_ = Transformation::none;
        weather_retry_started_.reset();
        ++revision_;
    }

}