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
        const RobinState& state,
        std::uint64_t now_ms) const noexcept {

        if (!can_start_transformation(state)) {
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
}