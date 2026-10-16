#include "pinkrobin/core.hpp"
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace pinkrobin;
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)

int main() {
    try {
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
        std::cout << "Core behavior, timing boundaries, interruption, isolation and clock validation passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
