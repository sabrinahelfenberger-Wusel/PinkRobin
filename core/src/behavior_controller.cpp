#include "pinkrobin/behavior_controller.hpp"

namespace pinkrobin {

    bool BehaviorController::can_start_transformation(
        const RobinState& state) const noexcept {

        return state.mode == Mode::awake &&
            state.mood == Mood::curious &&
            !state.head_contact &&
            state.transformation == Transformation::none;
    }

    bool BehaviorController::can_start_weather_reaction(
        const ActionProposal& proposal,
        const RobinState& state,
        ActionResource occupied,
        std::uint64_t now_ms) const noexcept {

        if (proposal.kind() != ActionKind::transformation) {
            return false;
        }

        if (!can_start_action(proposal, state, occupied)) {
            return false;
        }

        if (weather_reaction_started_) {
            if (now_ms < *weather_reaction_started_) {
                return false;
            }

            if (now_ms - *weather_reaction_started_ < 1800000) {
                return false;
            }
        }

        return true;
    }

    void BehaviorController::mark_weather_reaction_started(
        std::uint64_t now_ms) noexcept {

        weather_reaction_started_ = now_ms;
    }

    bool BehaviorController::can_start_action(
        const ActionProposal& proposal,
        const RobinState& state,
        ActionResource occupied) const noexcept {

        if (resources_conflict(
            proposal.required_resources(),
            occupied)) {
            return false;
        }

        if (const auto* transformation =
            std::get_if<TransformationAction>(&proposal.payload)) {

            if (transformation->transformation != Transformation::octopus &&
                transformation->transformation != Transformation::raincoat) {
                return false;
            }

            return can_start_transformation(state);
        }

        if (std::get_if<WeatherAnswerAction>(&proposal.payload)) {
            return true;
        }

        return false;
    }
}