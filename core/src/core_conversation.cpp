#include "pinkrobin/core.hpp"

#include <limits>

namespace pinkrobin {

    OutputEnqueueResult Core::process_text(
        const TextReceived& input) noexcept {

        if (!is_valid_text_input(input)) {
            return OutputEnqueueResult::invalid_argument;
        }

        if (!capabilities_.text_output) {
            return OutputEnqueueResult::unsupported;
        }

        const auto intent = recognize_text(
            input,
            conversation_.expected_reply);

        if (intent.kind == TextIntentKind::weather_question) {
            const auto observation = world_.weather();

            const bool has_selected_location =
                observation &&
                observation->location.kind ==
                WeatherLocationKind::selected_location;

            const auto result = has_selected_location
                ? request_weather_answer()
                : outputs_.enqueue(ConversationReply{
                    ConversationReplyKind::ask_postal_code
                    });

            if (result == OutputEnqueueResult::accepted) {
                conversation_.topic = ConversationTopic::weather;
                conversation_.expected_reply = has_selected_location
                    ? ExpectedReply::none
                    : ExpectedReply::swiss_postal_code;
                conversation_.last_input_at_ms = now_;
            }

            return result;
        }

        if (intent.kind == TextIntentKind::swiss_postal_code) {
            if (last_weather_request_id_ ==
                std::numeric_limits<std::uint64_t>::max()) {
                return OutputEnqueueResult::sequence_exhausted;
            }

            const auto request_id = last_weather_request_id_ + 1;

            const auto result = outputs_.enqueue(
                WeatherFetchRequested{
                    request_id,
                    intent.postal_code
                });

            if (result == OutputEnqueueResult::accepted) {
                last_weather_request_id_ = request_id;

                pending_weather_request_ = PendingWeatherRequest{
                    request_id,
                    intent.postal_code
                };

                conversation_.topic = ConversationTopic::weather;
                conversation_.expected_reply = ExpectedReply::none;
                conversation_.last_input_at_ms = now_;
            }

            return result;
        }

        const auto reply_kind =
            conversation_.expected_reply ==
            ExpectedReply::swiss_postal_code
            ? ConversationReplyKind::invalid_postal_code
            : ConversationReplyKind::unknown_input;

        const auto result = outputs_.enqueue(
            ConversationReply{ reply_kind });

        if (result == OutputEnqueueResult::accepted) {
            conversation_.last_input_at_ms = now_;
        }

        return result;
    }

}