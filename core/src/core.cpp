#include "pinkrobin/core.hpp"

namespace pinkrobin {
Core::Core(std::uint64_t now_ms) noexcept
    : now_(now_ms), greeting_started_(now_ms), interaction_started_(now_ms) {}

void Core::curious() noexcept {
    mode_ = Mode::awake;
    mood_ = Mood::curious;
    expression_ = Expression::curious;
    head_contact_ = false;
    heart_pending_ = false;
    ++revision_;
}

Result Core::advance_to(std::uint64_t now_ms) noexcept {
    if (now_ms < now_) return Result::time_reversed;
    now_ = now_ms;
    // Subtraction avoids overflow near the end of the uint64_t clock range.
    if (mode_ == Mode::sleeping && now_ - greeting_started_ >= 4000) {
        mode_ = Mode::waking;
        expression_ = Expression::curious;
        ++revision_;
    }
    if (mode_ == Mode::waking && now_ - greeting_started_ >= 7100) curious();
    if (head_contact_ && mood_ != Mood::affectionate && now_ - interaction_started_ >= 5000) {
        mood_ = Mood::affectionate;
        expression_ = Expression::heart_eyes;
        ++revision_;
    }
    if (heart_pending_ && now_ - interaction_started_ >= 6000) curious();
    return Result::ok;
}

Result Core::dispatch(Event event, std::uint64_t now_ms) noexcept {
    if (static_cast<std::uint32_t>(event) > static_cast<std::uint32_t>(Event::interaction_cancelled))
        return Result::invalid_argument;
    const auto result = advance_to(now_ms);
    if (result != Result::ok) return result;
    switch (event) {
    case Event::restart:
        mode_ = Mode::sleeping;
        mood_ = Mood::curious;
        expression_ = Expression::sleeping;
        head_contact_ = false;
        heart_pending_ = false;
        greeting_started_ = now_;
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
        // Pointer moves/repeated keydown must not postpone the five-second threshold.
        if (head_contact_) break;
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
        if (head_contact_) curious();
        break;
    }
    return Result::ok;
}

Snapshot Core::snapshot() const noexcept {
    return {mode_, mood_, expression_, head_contact_, now_, revision_};
}
}
