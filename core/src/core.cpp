#include "pinkrobin/core.hpp"
#include <cmath>

namespace pinkrobin {

    Core::Core(
        std::uint64_t now_ms,
        PlatformCapabilities capabilities) noexcept
        : now_(now_ms),
        greeting_started_(now_ms),
        interaction_started_(now_ms),
        capabilities_(capabilities) {}

    Result Core::advance_state_to(std::uint64_t now_ms) noexcept {
        if (now_ms < now_) {
            return Result::time_reversed;
        }

        now_ = now_ms;
        if (conversation_.expire(now_)) {
            pending_weather_request_.reset();
        }

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
                    pending_weather_received_at_ms_,
                    pending_weather_->location
                });

                pending_weather_.reset();
            }
        }

        // Abgelaufenes Wetterwissen entfernen.
        world_.expire_weather(now_);

        // Subtraktion vermeidet Überläufe am Ende des Zeitbereichs.
        if (state_.mode == Mode::sleeping &&
            now_ - greeting_started_ >= 4000) {
            state_.mode = Mode::waking;
            state_.expression = Expression::curious;
            ++revision_;
        }

        if (state_.mode == Mode::waking &&
            now_ - greeting_started_ >= 7100) {
            curious();
        }

        if (state_.head_contact &&
            state_.mood != Mood::affectionate &&
            now_ - interaction_started_ >= 5000) {
            state_.mood = Mood::affectionate;
            state_.expression = Expression::heart_eyes;
            ++revision_;
        }

        if (heart_pending_ &&
            now_ - interaction_started_ >= 6000) {
            curious();
        }

        // Aktive Verwandlung nach zehn Sekunden beenden.
        if (state_.transformation != Transformation::none &&
            now_ - transformation_started_ >= 10000) {
            state_.transformation = Transformation::none;
            ++revision_;
        }

        process_weather_reaction();

        return Result::ok;
    }

    Result Core::advance_to(std::uint64_t now_ms) noexcept {
        if (now_ms < now_) {
            return Result::time_reversed;
        }

        while (input_count_ > 0) {
            const auto& pending = pending_inputs_[input_head_];

            if (pending.received_at_ms > now_ms) {
                break;
            }

            const auto processing_time =
                pending.received_at_ms < now_
                ? now_
                : pending.received_at_ms;

            const auto result = advance_state_to(processing_time);

            if (result != Result::ok) {
                return result;
            }

            auto output_result = OutputEnqueueResult::accepted;

            if (const auto* text =
                std::get_if<TextReceived>(&pending.payload)) {

                output_result = process_text(*text);
            }
            else if (const auto* weather_result =
                std::get_if<WeatherFetchResult>(&pending.payload)) {

                output_result = process_weather_result(*weather_result);
            }
            else if (const auto* interaction =
                std::get_if<InteractionReceived>(&pending.payload)) {

                apply_interaction(interaction->event);
            }

            if (output_result == OutputEnqueueResult::full) {
                break;
            }

            if (output_result == OutputEnqueueResult::unsupported) {
                return Result::unsupported;
            }

            if (output_result == OutputEnqueueResult::sequence_exhausted) {
                return Result::sequence_exhausted;
            }

            if (output_result != OutputEnqueueResult::accepted) {
                return Result::invalid_argument;
            }

            dequeue_input(processing_time);
        }

        return advance_state_to(now_ms);
    }

    bool Core::can_start_transformation() const noexcept {
        return behavior_controller_.can_start_transformation(state_);
    }

    Result Core::start_transformation(
        Transformation transformation,
        std::uint64_t now_ms) noexcept {

        if (transformation != Transformation::octopus &&
            transformation != Transformation::raincoat) {
            return Result::invalid_argument;
        }

        const ActionProposal proposal{
            TransformationAction{ transformation }
        };

        if (!supports_action(capabilities_, proposal)) {
            return Result::unsupported;
        }

        const auto result = advance_to(now_ms);

        if (result != Result::ok) {
            return result;
        }

        if (!can_start_transformation()) {
            return Result::busy;
        }

        state_.transformation = transformation;
        transformation_started_ = now_;
        ++revision_;

        return Result::ok;
    }

    Snapshot Core::snapshot() const noexcept {
        return {
            state_.mode,
            state_.mood,
            state_.expression,
            state_.head_contact,
            now_,
            revision_,
            state_.transformation
        };
    }

    const WorldModel& Core::world() const noexcept {
        return world_;
    }

    const RobinState& Core::state() const noexcept {
        return state_;
    }

    const ConversationState& Core::conversation() const noexcept {
        return conversation_;
    }
}