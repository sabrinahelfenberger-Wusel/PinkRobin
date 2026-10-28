#include "pinkrobin/core.hpp"

namespace pinkrobin {
    void Core::curious() noexcept {
        mode_ = Mode::awake;
        mood_ = Mood::curious;
        expression_ = Expression::curious;
        head_contact_ = false;
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
            mode_ = Mode::sleeping;
            mood_ = Mood::curious;
            expression_ = Expression::sleeping;
            head_contact_ = false;
            heart_pending_ = false;
            greeting_started_ = now_;
            transformation_ = Transformation::none;
            ++revision_;
            break;

        case Event::heart_touched:
            mode_ = Mode::awake;
            mood_ = Mood::happy;
            expression_ = Expression::happy;
            head_contact_ = false;
            heart_pending_ = true;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_started:
            // Wiederholte Eingaben verschieben die Zeitgrenze nicht.
            if (head_contact_) {
                break;
            }

            mode_ = Mode::awake;
            mood_ = Mood::happy;
            expression_ = Expression::happy;
            head_contact_ = true;
            heart_pending_ = false;
            interaction_started_ = now_;
            ++revision_;
            break;

        case Event::head_ended:
        case Event::interaction_cancelled:
            if (head_contact_) {
                curious();
            }
            break;
        }
    }

}