#include "pinkrobin/core.hpp"
#include <cmath>

namespace pinkrobin {

    Core::Core(std::uint64_t now_ms) noexcept
        : now_(now_ms),
        greeting_started_(now_ms),
        interaction_started_(now_ms) {}

    Result Core::advance_state_to(std::uint64_t now_ms) noexcept {
        if (now_ms < now_) {
            return Result::time_reversed;
        }

        now_ = now_ms;

        // Vorgemerkte Wetterprognose verarbeiten.
        if (pending_weather_ && pending_weather_received_at_ms_ <= now_) {
            const auto& forecast = pending_weather_->forecast;

            if (forecast.valid_until_ms <= now_) {
                pending_weather_.reset();
            }
            else if (forecast.valid_from_ms <= now_) {
                world_.update_weather(WeatherObservation{
                    forecast,
                    pending_weather_source_,
                    pending_weather_received_at_ms_
                    });

                pending_weather_.reset();
            }
        }

        // Abgelaufenes Wetterwissen entfernen.
        world_.expire_weather(now_);

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

    Result Core::advance_to(std::uint64_t now_ms) noexcept {
        if (now_ms < now_) {
            return Result::time_reversed;
        }

        while (interaction_count_ > 0) {
            const auto event_time =
                pending_interactions_[interaction_head_].received_at_ms;

            if (event_time > now_ms) {
                break;
            }

            const auto result = advance_state_to(event_time);

            if (result != Result::ok) {
                return result;
            }

            const auto pending = dequeue_interaction(event_time);

            if (pending) {
                apply_interaction(pending->interaction.event);
            }
        }

        return advance_state_to(now_ms);
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

    const WorldModel& Core::world() const noexcept {
        return world_;
    }

}