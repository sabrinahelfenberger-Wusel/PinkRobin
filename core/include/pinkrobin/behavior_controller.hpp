#pragma once

#include <cstdint>
#include <optional>
#include "pinkrobin/robin_state.hpp"
#include "pinkrobin/action.hpp"

namespace pinkrobin {

    class BehaviorController final {
    public:
        bool can_start_transformation(
            const RobinState& state) const noexcept;

        bool can_start_weather_reaction(
            const ActionProposal& proposal,
            const RobinState& state,
            ActionResource occupied,
            std::uint64_t now_ms) const noexcept;

        void mark_weather_reaction_started(
            std::uint64_t now_ms) noexcept;

        bool can_start_action(
            const ActionProposal& proposal,
            const RobinState& state,
            ActionResource occupied) const noexcept;

    private:
        std::optional<std::uint64_t> weather_reaction_started_;
    };
}