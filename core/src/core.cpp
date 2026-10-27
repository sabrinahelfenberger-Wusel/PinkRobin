#include "pinkrobin/core.hpp"
#include <cmath>

namespace pinkrobin {

    Core::Core(std::uint64_t now_ms) noexcept
        : now_(now_ms),
        greeting_started_(now_ms),
        interaction_started_(now_ms) {}

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