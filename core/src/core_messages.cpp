#include "pinkrobin/core.hpp"

namespace pinkrobin {
    MessageAcceptance Core::receive(
        const Message& message,
        std::uint64_t now_ms) noexcept {

        switch (message.metadata.source) {
        case InputSource::web:
        case InputSource::companion:
        case InputSource::device:
        case InputSource::backend:
            break;

        default:
            return MessageAcceptance::invalid_argument;
        }

        if (now_ms < now_ ||
            now_ms < last_message_received_at_ms_) {
            return MessageAcceptance::time_reversed;
        }

        if (const auto* interaction =
            std::get_if<InteractionReceived>(&message.payload)) {

            const auto acceptance = enqueue_input(
                message.metadata,
                *interaction,
                now_ms);

            if (acceptance == MessageAcceptance::accepted) {
                last_message_received_at_ms_ = now_ms;
            }

            return acceptance;
        }

        if (const auto* text =
            std::get_if<TextReceived>(&message.payload)) {

            if (!is_valid_text_input(*text)) {
                return MessageAcceptance::invalid_argument;
            }

            if (!capabilities_.text_output) {
                return MessageAcceptance::unsupported;
            }

            const auto acceptance = enqueue_input(
                message.metadata,
                *text,
                now_ms);

            if (acceptance == MessageAcceptance::accepted) {
                last_message_received_at_ms_ = now_ms;
            }

            return acceptance;
        }

        if (const auto* weather =
            std::get_if<WeatherUpdated>(&message.payload)) {

            if (!is_valid_weather_location(weather->location)) {
                return MessageAcceptance::invalid_argument;
            }

            if (validate_weather(weather->forecast, now_ms)
                != Result::ok) {
                return MessageAcceptance::invalid_argument;
            }

            const bool replaces_pending =
                pending_weather_.has_value();

            pending_weather_ = *weather;
            pending_weather_source_ = message.metadata.source;
            pending_weather_received_at_ms_ = now_ms;
            last_message_received_at_ms_ = now_ms;

            return replaces_pending
                ? MessageAcceptance::replaced
                : MessageAcceptance::accepted;
        }

        return MessageAcceptance::invalid_argument;
    }

    MessageAcceptance Core::enqueue_input(
        const MessageMetadata& metadata,
        const InputPayload& payload,
        std::uint64_t received_at_ms) noexcept {

        if (const auto* interaction =
            std::get_if<InteractionReceived>(&payload)) {

            if (static_cast<std::uint32_t>(interaction->event) >
                static_cast<std::uint32_t>(Event::interaction_cancelled)) {
                return MessageAcceptance::invalid_argument;
            }
        }
        else if (const auto* text =
            std::get_if<TextReceived>(&payload)) {

            if (!is_valid_text_input(*text)) {
                return MessageAcceptance::invalid_argument;
            }
        }
        else {
            return MessageAcceptance::invalid_argument;
        }

        if (received_at_ms < now_) {
            return MessageAcceptance::time_reversed;
        }

        if (input_count_ > 0) {
            const auto last_position =
                (input_head_ + input_count_ - 1) % input_capacity;

            if (received_at_ms <
                pending_inputs_[last_position].received_at_ms) {
                return MessageAcceptance::time_reversed;
            }
        }

        if (input_count_ == input_capacity) {
            return MessageAcceptance::full;
        }

        const auto position =
            (input_head_ + input_count_) % input_capacity;

        pending_inputs_[position] = PendingInput{
            metadata,
            payload,
            received_at_ms
        };

        ++input_count_;

        return MessageAcceptance::accepted;
    }

    std::optional<Core::PendingInput>
        Core::dequeue_input(std::uint64_t now_ms) noexcept {

        if (input_count_ == 0) {
            return std::nullopt;
        }

        if (pending_inputs_[input_head_].received_at_ms > now_ms) {
            return std::nullopt;
        }

        const auto interaction =
            pending_inputs_[input_head_];

        input_head_ =
            (input_head_ + 1) % input_capacity;

        --input_count_;

        return interaction;
    }

}