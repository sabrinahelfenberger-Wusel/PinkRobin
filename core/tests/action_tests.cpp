#include "pinkrobin/action.hpp"
#include "pinkrobin/behavior_controller.hpp"
#include "test_support.hpp"

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
            outfit, state, ActionResource::none));

        // Eine Antwort darf parallel erfolgen.
        CHECK(controller.can_start_action(
            answer, state, ActionResource::none));

        // Ein belegtes Outfit blockiert die Textantwort nicht.
        CHECK(controller.can_start_action(
            answer, state, ActionResource::outfit));

        // Eine bereits belegte Textausgabe blockiert sie.
        CHECK(!controller.can_start_action(
            answer, state, ActionResource::text_output));

        state.head_contact = false;

        CHECK(controller.can_start_action(
            outfit, state, ActionResource::none));

        CHECK(!controller.can_start_action(
            outfit, state, ActionResource::outfit));
    }

}

int main() {
    return test_support::run_tests({
        { "action: proposal_properties", test_action_proposal_properties },
        { "action: availability", test_action_availability },
    });
}