#include "pinkrobin/action.hpp"
#include "pinkrobin/behavior_controller.hpp"
#include "test_support.hpp"
#include "pinkrobin/core.hpp"

using namespace pinkrobin;

namespace {

    void test_action_proposal_properties() {
        const ActionProposal outfit{
            TransformationAction{ Transformation::raincoat }
        };

        CHECK(outfit.kind() == ActionKind::transformation);
        CHECK(
            outfit.required_resources()
            == ActionResource::outfit
        );

        const ActionProposal answer{
            WeatherAnswerAction{}
        };

        CHECK(answer.kind() == ActionKind::weather_answer);
        CHECK(
            answer.required_resources()
            == ActionResource::text_output
        );

        CHECK(resources_conflict(
            ActionResource::outfit,
            ActionResource::outfit));

        CHECK(!resources_conflict(
            ActionResource::text_output,
            ActionResource::outfit));

        CHECK(!resources_conflict(
            ActionResource::outfit,
            ActionResource::none));
    }

    void test_action_availability() {
        const PlatformCapabilities capabilities{ true, true };
        BehaviorController controller;
        RobinState state;
        state.mode = Mode::awake;
        state.head_contact = true;

        const ActionProposal outfit{
            TransformationAction{ Transformation::raincoat }
        };

        const ActionProposal answer{
            WeatherAnswerAction{}
        };

        // Outfitwechsel wartet während des Streichelns.
        CHECK(!controller.can_start_action(
            capabilities, outfit, state, ActionResource::none));

        // Eine Antwort darf parallel erfolgen.
        CHECK(controller.can_start_action(
            capabilities, answer, state, ActionResource::none));

        // Ein belegtes Outfit blockiert die Textantwort nicht.
        CHECK(controller.can_start_action(
            capabilities, answer, state, ActionResource::outfit));

        // Eine bereits belegte Textausgabe blockiert sie.
        CHECK(!controller.can_start_action(
            capabilities, answer, state, ActionResource::text_output));

        state.head_contact = false;

        CHECK(controller.can_start_action(
            capabilities, outfit, state, ActionResource::none));

        CHECK(!controller.can_start_action(
            capabilities, outfit, state, ActionResource::outfit));
    }

    void test_platform_capabilities() {
        const ActionProposal outfit{
            TransformationAction{ Transformation::raincoat }
        };

        const ActionProposal answer{
            WeatherAnswerAction{}
        };

        const PlatformCapabilities unavailable{};

        CHECK(!supports_action(unavailable, outfit));
        CHECK(!supports_action(unavailable, answer));

        const PlatformCapabilities text_only{
            true, false
        };

        CHECK(supports_action(text_only, answer));
        CHECK(!supports_action(text_only, outfit));

        const PlatformCapabilities outfit_only{
            false, true
        };

        CHECK(!supports_action(outfit_only, answer));
        CHECK(supports_action(outfit_only, outfit));

        const PlatformCapabilities both{
            true, true
        };

        CHECK(supports_action(both, answer));
        CHECK(supports_action(both, outfit));
    }

    void test_core_without_capabilities() {
        Core core;

        CHECK(core.advance_to(8000) == Result::ok);

        const auto before = core.snapshot();

        CHECK(core.start_transformation(
            Transformation::raincoat,
            8000) == Result::unsupported);

        CHECK(core.request_weather_answer() ==
            OutputEnqueueResult::unsupported);

        CHECK(!core.next_output().has_value());

        const auto after = core.snapshot();

        CHECK(after.transformation == Transformation::none);
        CHECK(after.revision == before.revision);
        CHECK(after.time_ms == before.time_ms);
    }

    void test_core_with_partial_capabilities() {
        Core text_only{
            0, PlatformCapabilities{ true, false }
        };

        CHECK(text_only.advance_to(8000) == Result::ok);

        CHECK(text_only.start_transformation(
            Transformation::raincoat,
            8000) == Result::unsupported);

        CHECK(text_only.request_weather_answer() ==
            OutputEnqueueResult::accepted);

        CHECK(text_only.next_output().has_value());

        Core outfit_only{
            0, PlatformCapabilities{ false, true }
        };

        CHECK(outfit_only.advance_to(8000) == Result::ok);

        CHECK(outfit_only.start_transformation(
            Transformation::raincoat,
            8000) == Result::ok);

        CHECK(outfit_only.snapshot().transformation ==
            Transformation::raincoat);

        CHECK(outfit_only.request_weather_answer() ==
            OutputEnqueueResult::unsupported);

        CHECK(!outfit_only.next_output().has_value());
    }
}

int main() {
    return test_support::run_tests({
        { "action: proposal_properties", test_action_proposal_properties },
        { "action: availability", test_action_availability },
        { "action: platform_capabilities", test_platform_capabilities },
        { "action: core_without_capabilities", test_core_without_capabilities },
        { "action: core_with_partial_capabilities", test_core_with_partial_capabilities },
    });
}