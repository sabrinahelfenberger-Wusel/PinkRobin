#include "pinkrobin/core.hpp"
#include <cmath>

namespace pinkrobin {

    Core::Core(std::uint64_t now_ms) noexcept
        : now_(now_ms),
        greeting_started_(now_ms),
        interaction_started_(now_ms) {}

    void Core::curious() noexcept {
        mode_ = Mode::awake;
        mood_ = Mood::curious;
        expression_ = Expression::curious;
        head_contact_ = false;
        heart_pending_ = false;
        ++revision_;
    }

    Result Core::advance_to(std::uint64_t now_ms) noexcept {
        if (now_ms < now_) {
            return Result::time_reversed;
        }

        now_ = now_ms;

        // Vorgemerkte Wetterprognose verarbeiten.
        if (pending_weather_) {
            const auto& forecast = pending_weather_->forecast;

            if (forecast.valid_until_ms <= now_) {
                pending_weather_.reset();
            }
            else if (forecast.valid_from_ms <= now_) {
                weather_ = forecast;
                pending_weather_.reset();
            }
        }

        // Abgelaufenes Wetterwissen entfernen.
        if (weather_ && weather_->valid_until_ms <= now_) {
            weather_.reset();
        }

        // Subtraktion vermeidet Überläufe am Ende des Zeitbereichs.
        if (mode_ == Mode::sleeping &&
            now_ - greeting_started_ >= 4000) {
            mode_ = Mode::waking;
            expression_ = Expression::curious;
            ++revision_;
        }

        if (mode_ == Mode::waking &&
            now_ - greeting_started_ >= 7100) {
            curious();
        }

        if (head_contact_ &&
            mood_ != Mood::affectionate &&
            now_ - interaction_started_ >= 5000) {
            mood_ = Mood::affectionate;
            expression_ = Expression::heart_eyes;
            ++revision_;
        }

        if (heart_pending_ &&
            now_ - interaction_started_ >= 6000) {
            curious();
        }

        // Aktive Verwandlung nach zehn Sekunden beenden.
        if (transformation_ != Transformation::none &&
            now_ - transformation_started_ >= 10000) {
            transformation_ = Transformation::none;
            ++revision_;
        }

        process_weather_reaction();

        return Result::ok;
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

    bool Core::can_start_transformation() const noexcept {
        return mode_ == Mode::awake &&
            mood_ == Mood::curious &&
            !head_contact_ &&
            transformation_ == Transformation::none;
    }

    Result Core::start_transformation(
        Transformation transformation,
        std::uint64_t now_ms) noexcept {

        if (transformation != Transformation::octopus &&
            transformation != Transformation::raincoat) {
            return Result::invalid_argument;
        }

        const auto result = advance_to(now_ms);

        if (result != Result::ok) {
            return result;
        }

        if (!can_start_transformation()) {
            return Result::busy;
        }

        transformation_ = transformation;
        transformation_started_ = now_;
        ++revision_;

        return Result::ok;
    }

    Result Core::dispatch(
        Event event,
        std::uint64_t now_ms) noexcept {

        if (static_cast<std::uint32_t>(event) >
            static_cast<std::uint32_t>(Event::interaction_cancelled)) {
            return Result::invalid_argument;
        }

        const auto result = advance_to(now_ms);

        if (result != Result::ok) {
            return result;
        }

        switch (event) {
        case Event::restart:
            mode_ = Mode::sleeping;
            mood_ = Mood::curious;
            expression_ = Expression::sleeping;
            head_contact_ = false;
            heart_pending_ = false;
            greeting_started_ = now_;
            transformation_ = Transformation::none;
            ++revision_;
            break;

        case Event::heart_touched:
            mode_ = Mode::awake;
            mood_ = Mood::happy;
            expression_ = Expression::happy;
            head_contact_ = false;
            heart_pending_ = true;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_started:
            // Wiederholte Eingaben verschieben die Zeitgrenze nicht.
            if (head_contact_) {
                break;
            }

            mode_ = Mode::awake;
            mood_ = Mood::happy;
            expression_ = Expression::happy;
            head_contact_ = true;
            heart_pending_ = false;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_ended:
        case Event::interaction_cancelled:
            if (head_contact_) {
                curious();
            }
            break;
        }

        return Result::ok;
    }

    Result Core::post_weather(
        const WeatherUpdated& message) noexcept {

        const auto& forecast = message.forecast;

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
            forecast.valid_until_ms <= now_) {
            return Result::invalid_argument;
        }

        pending_weather_ = message;

        return Result::ok;
    }

    std::optional<WeatherForecast> Core::weather() const noexcept {
        return weather_;
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

    Snapshot Core::snapshot() const noexcept {
        return {
            mode_,
            mood_,
            expression_,
            head_contact_,
            now_,
            revision_,
            transformation_
        };
    }

}