#include "pinkrobin/core.hpp"

namespace pinkrobin {
    void Core::curious() noexcept {
        state_.mode = Mode::awake;
        mood_ = Mood::curious;
        state_.expression = Expression::curious;
        state_.head_contact = false;
        heart_pending_ = false;
        ++revision_;
    }

    Result Core::dispatch(
        Event event,
        std::uint64_t now_ms) noexcept {

        if (static_cast<std::uint32_t>(event) >
            static_cast<std::uint32_t>(Event::interaction_cancelled)) {
            return Result::invalid_argument;
        }

        const auto result = advance_to(now_ms);

        if (result != Result::ok) {
            return result;
        }

        apply_interaction(event);

        return Result::ok;
    }

    void Core::apply_interaction(Event event) noexcept {
        
        switch (event) {
        case Event::restart:
            state_.mode = Mode::sleeping;
            mood_ = Mood::curious;
            state_.expression = Expression::sleeping;
            state_.head_contact = false;
            heart_pending_ = false;
            greeting_started_ = now_;
            state_.transformation = Transformation::none;
            ++revision_;
            break;

        case Event::heart_touched:
            state_.mode = Mode::awake;
            mood_ = Mood::happy;
            state_.expression = Expression::happy;
            state_.head_contact = false;
            heart_pending_ = true;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_started:
            // Wiederholte Eingaben verschieben die Zeitgrenze nicht.
            if (state_.head_contact) {
                break;
            }

            state_.mode = Mode::awake;
            mood_ = Mood::happy;
            state_.expression = Expression::happy;
            state_.head_contact = true;
            heart_pending_ = false;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_ended:
        case Event::interaction_cancelled:
            if (state_.head_contact) {
                curious();
            }
            break;
        }
    }

}