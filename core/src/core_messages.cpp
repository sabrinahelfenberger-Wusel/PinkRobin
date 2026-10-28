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

            const auto acceptance = enqueue_interaction(
                message.metadata,
                *interaction,
                now_ms);

            if (acceptance == MessageAcceptance::accepted) {
                last_message_received_at_ms_ = now_ms;
            }

            return acceptance;
        }

        if (const auto* weather =
            std::get_if<WeatherUpdated>(&message.payload)) {

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

    MessageAcceptance Core::enqueue_interaction(
        const MessageMetadata& metadata,
        const InteractionReceived& interaction,
        std::uint64_t received_at_ms) noexcept {

        if (static_cast<std::uint32_t>(interaction.event) >
            static_cast<std::uint32_t>(Event::interaction_cancelled)) {
            return MessageAcceptance::invalid_argument;
        }

        if (received_at_ms < now_) {
            return MessageAcceptance::time_reversed;
        }

        if (interaction_count_ > 0) {
            const auto last_position =
                (interaction_head_ + interaction_count_ - 1)
                % interaction_capacity;

            if (received_at_ms <
                pending_interactions_[last_position].received_at_ms) {
                return MessageAcceptance::time_reversed;
            }
        }

        if (interaction_count_ == interaction_capacity) {
            return MessageAcceptance::full;
        }

        const auto position =
            (interaction_head_ + interaction_count_)
            % interaction_capacity;

        pending_interactions_[position] = PendingInteraction{
            metadata,
            interaction,
            received_at_ms
        };

        ++interaction_count_;

        return MessageAcceptance::accepted;
    }

    std::optional<Core::PendingInteraction>
        Core::dequeue_interaction(std::uint64_t now_ms) noexcept {

        if (interaction_count_ == 0) {
            return std::nullopt;
        }

        if (pending_interactions_[interaction_head_].received_at_ms > now_ms) {
            return std::nullopt;
        }

        const auto interaction =
            pending_interactions_[interaction_head_];

        interaction_head_ =
            (interaction_head_ + 1) % interaction_capacity;

        --interaction_count_;

        return interaction;
    }

}