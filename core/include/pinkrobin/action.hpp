#pragma once

#include <cstdint>
#include <variant>
#include "pinkrobin/robin_state.hpp"

namespace pinkrobin {

    enum class ActionKind : std::uint32_t {
        transformation = 0,
        weather_answer = 1
    };

    enum class ActionResource : std::uint32_t {
        none = 0,
        outfit = 1u << 0,
        text_output = 1u << 1
    };

    constexpr bool resources_conflict(
        ActionResource required,
        ActionResource occupied) noexcept {

        return (
            static_cast<std::uint32_t>(required) &
            static_cast<std::uint32_t>(occupied)
            ) != 0;
    }

    struct TransformationAction {
        static constexpr ActionKind kind =
            ActionKind::transformation;

        static constexpr ActionResource resources =
            ActionResource::outfit;

        Transformation transformation;
    };

    struct WeatherAnswerAction {
        static constexpr ActionKind kind =
            ActionKind::weather_answer;

        static constexpr ActionResource resources =
            ActionResource::text_output;
    };

    using ActionPayload = std::variant<
        TransformationAction,
        WeatherAnswerAction
    >;

    struct ActionProposal {
        ActionPayload payload;

        ActionKind kind() const noexcept {
            return std::visit(
                [](const auto& action) {
                    return action.kind;
                },
                payload);
        }

        ActionResource required_resources() const noexcept {
            return std::visit(
                [](const auto& action) {
                    return action.resources;
                },
                payload);
        }
    };
}