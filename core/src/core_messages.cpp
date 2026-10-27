#include "pinkrobin/core.hpp"

namespace pinkrobin {
    Result Core::receive(
        const Message& message,
        std::uint64_t now_ms) noexcept {

        switch (message.metadata.source) {
        case InputSource::web:
        case InputSource::companion:
        case InputSource::device:
            break;

        default:
            return Result::invalid_argument;
        }

        if (now_ms < now_) {
            return Result::time_reversed;
        }

        if (const auto* interaction = std::get_if<InteractionReceived>(&message.payload)) {
            return dispatch(interaction->event, now_ms);
        }

        if (const auto* weather =
            std::get_if<WeatherUpdated>(&message.payload)) {

            const auto result =
                validate_weather(weather->forecast, now_ms);

            if (result != Result::ok) {
                return result;
            }

            pending_weather_ = *weather;

            return advance_to(now_ms);
        }

        return Result::invalid_argument;
    }

}