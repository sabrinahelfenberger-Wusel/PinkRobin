#include "pinkrobin/core.hpp"
#include "pinkrobin/behavior_controller.hpp"
#include "pinkrobin/action.hpp"
#include "test_support.hpp"
#include <cstdint>
#include <limits>

using namespace pinkrobin;

namespace {

void test_greeting() {
    Core greeting;
    CHECK(greeting.snapshot().mode == Mode::sleeping);
    CHECK(greeting.advance_to(3999) == Result::ok);
    CHECK(greeting.snapshot().expression == Expression::sleeping);
    greeting.advance_to(4000);
    CHECK(greeting.snapshot().mode == Mode::waking);
    greeting.advance_to(7099);
    CHECK(greeting.snapshot().mode == Mode::waking);
    greeting.advance_to(7100);
    CHECK(greeting.snapshot().mode == Mode::awake);
    CHECK(greeting.snapshot().mood == Mood::curious);
    Core jumped;
    jumped.advance_to(1000000);
    CHECK(jumped.snapshot().mode == Mode::awake);
    CHECK(jumped.snapshot().revision == greeting.snapshot().revision);
}

void test_heart() {
    Core heart;
    heart.dispatch(Event::heart_touched, 500);
    CHECK(heart.snapshot().expression == Expression::happy);
    heart.advance_to(6499);
    CHECK(heart.snapshot().mood == Mood::happy);
    heart.dispatch(Event::heart_touched, 6499);
    heart.advance_to(6500);
    CHECK(heart.snapshot().mood == Mood::happy);
    heart.advance_to(12499);
    CHECK(heart.snapshot().mood == Mood::curious);
    CHECK(heart.snapshot().mode == Mode::awake);
}

void test_head_contact() {
    Core head;
    head.dispatch(Event::head_started, 0);
    head.advance_to(4999);
    CHECK(head.snapshot().expression == Expression::happy);
    const auto revision = head.snapshot().revision;
    head.dispatch(Event::head_started, 4999);
    CHECK(head.snapshot().revision == revision);
    head.advance_to(5000);
    CHECK(head.snapshot().expression == Expression::heart_eyes);
    CHECK(head.snapshot().head_contact);
    head.advance_to(100000);
    CHECK(head.snapshot().expression == Expression::heart_eyes);
    head.dispatch(Event::head_ended, 100000);
    CHECK(head.snapshot().expression == Expression::curious);
    CHECK(!head.snapshot().head_contact);
    head.dispatch(Event::head_started, 100001);
    head.dispatch(Event::interaction_cancelled, 100002);
    head.advance_to(200000);
    CHECK(head.snapshot().expression == Expression::curious);
    head.dispatch(Event::head_started, 200000);
    head.dispatch(Event::heart_touched, 200001);
    head.dispatch(Event::head_ended, 200002);
    CHECK(head.snapshot().expression == Expression::happy);
    CHECK(!head.snapshot().head_contact);
    head.advance_to(206001);
    CHECK(head.snapshot().expression == Expression::curious);
}

void test_restart_and_invalid_input() {
    Core reset;
    reset.dispatch(Event::head_started, 10);
    reset.dispatch(Event::restart, 1000);
    reset.advance_to(4999);
    CHECK(reset.snapshot().mode == Mode::sleeping);
    reset.advance_to(5000);
    CHECK(reset.snapshot().mode == Mode::waking);
    CHECK(!reset.snapshot().head_contact);
    reset.advance_to(8100);
    CHECK(reset.snapshot().mood == Mood::curious);
    auto before = reset.snapshot();
    CHECK(reset.advance_to(8099) == Result::time_reversed);
    CHECK(reset.dispatch(Event::heart_touched, 8099) == Result::time_reversed);
    CHECK(reset.dispatch(static_cast<Event>(999), 9000) == Result::invalid_argument);
    CHECK(reset.snapshot().time_ms == before.time_ms);
    CHECK(reset.snapshot().revision == before.revision);
}

void test_isolation_and_clock_limits() {
    Core isolated_a, isolated_b;
    isolated_a.dispatch(Event::head_started, 0);
    isolated_a.advance_to(5000);
    CHECK(isolated_b.snapshot().mode == Mode::sleeping);
    CHECK(isolated_b.snapshot().revision == 0);
    CHECK(isolated_a.snapshot().expression == Expression::heart_eyes);
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    Core near_end(maximum - 5000);
    near_end.dispatch(Event::heart_touched, maximum - 5000);
    near_end.advance_to(maximum);
    CHECK(near_end.snapshot().mood == Mood::happy);
    CHECK(near_end.advance_to(0) == Result::time_reversed);
}

void test_robin_state_access() {
    Core core;

    CHECK(core.state().mode == Mode::sleeping);
    CHECK(core.state().mood == Mood::curious);
    CHECK(core.state().expression == Expression::sleeping);
    CHECK(!core.state().head_contact);
    CHECK(core.state().transformation == Transformation::none);

    CHECK(
        core.dispatch(Event::heart_touched, 100)
        == Result::ok
    );

    CHECK(core.state().mode == Mode::awake);
    CHECK(core.state().mood == Mood::happy);
    CHECK(core.state().expression == Expression::happy);

    const auto snapshot = core.snapshot();

    CHECK(snapshot.mode == core.state().mode);
    CHECK(snapshot.mood == core.state().mood);
    CHECK(snapshot.expression == core.state().expression);
    CHECK(snapshot.head_contact == core.state().head_contact);
    CHECK(snapshot.transformation == core.state().transformation);
}

void test_behavior_controller_availability() {
    BehaviorController controller;
    RobinState state;

    // Schlafend: keine Verwandlung.
    CHECK(!controller.can_start_transformation(state));

    state.mode = Mode::awake;
    CHECK(controller.can_start_transformation(state));

    // Freude nach einer Interaktion ist noch aktiv.
    state.mood = Mood::happy;
    CHECK(!controller.can_start_transformation(state));

    state.mood = Mood::curious;
    state.head_contact = true;
    CHECK(!controller.can_start_transformation(state));

    state.head_contact = false;
    state.transformation = Transformation::octopus;
    CHECK(!controller.can_start_transformation(state));

    // Wieder frei.
    state.transformation = Transformation::none;
    CHECK(controller.can_start_transformation(state));
}

void test_behavior_controller_weather_cooldown() {
    BehaviorController controller;
    RobinState state;
    state.mode = Mode::awake;

    const ActionProposal proposal{
        TransformationAction{ Transformation::raincoat }
    };

    CHECK(controller.can_start_weather_reaction(proposal, state, ActionResource::none, 8000));

    // Eine reine Abfrage verbraucht keine Freigabe.
    CHECK(controller.can_start_weather_reaction(proposal, state, ActionResource::none, 8001));

    controller.mark_weather_reaction_started(8000);

    // Rückwärtszeit darf den Cooldown nicht umgehen.
    CHECK(!controller.can_start_weather_reaction(proposal, state, ActionResource::none, 7999));

    // Eine Millisekunde vor Ablauf der 30 Minuten.
    CHECK(!controller.can_start_weather_reaction(proposal, state, ActionResource::none, 1807999));

    // Genau am Ende ist ein neuer Start erlaubt.
    CHECK(controller.can_start_weather_reaction(proposal, state, ActionResource::none, 1808000));

    // Ein abgelaufener Cooldown macht einen beschäftigten Robin nicht frei.
    state.head_contact = true;
    CHECK(!controller.can_start_weather_reaction(proposal, state, ActionResource::none, 1808000));

    state.head_contact = false;
    CHECK(controller.can_start_weather_reaction(proposal, state, ActionResource::none, 1808000));
}

} // namespace

int main() {
    return test_support::run_tests({
        {"core: greeting", test_greeting},
        {"core: heart", test_heart},
        {"core: head_contact", test_head_contact},
        {"core: restart_and_invalid_input", test_restart_and_invalid_input},
        {"core: isolation_and_clock_limits", test_isolation_and_clock_limits},
        {"core: robin_state_access", test_robin_state_access},
        {"core: behavior_controller_availability", test_behavior_controller_availability},
        {"core: behavior_controller_weather_cooldown", test_behavior_controller_weather_cooldown},
    });
}
