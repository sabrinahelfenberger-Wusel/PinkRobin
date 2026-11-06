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

    OutputEnqueueResult Core::process_weather_result(
        const WeatherFetchResult& result) noexcept {

        // Alte oder bereits erledigte Anfragen erzeugen keine Ausgabe.
        if (!pending_weather_request_ ||
            pending_weather_request_->request_id != result.request_id) {
            return OutputEnqueueResult::accepted;
        }

        const bool usable =
            result.forecast &&
            validate_weather(*result.forecast, now_) == Result::ok &&
            result.forecast->valid_from_ms <= now_;

        OutputPayload payload = ConversationReply{
            ConversationReplyKind::weather_unavailable
        };

        if (usable) {
            const auto& forecast = *result.forecast;

            auto kind = WeatherAnswerKind::dry;

            if (forecast.precipitation_mm >= 10.0 &&
                forecast.wind_gust_kmh >= 60.0) {
                kind = WeatherAnswerKind::storm;
            }
            else if (forecast.precipitation_mm > 0.0) {
                kind = WeatherAnswerKind::rain;
            }

            payload = WeatherAnswer{ kind };
        }

        const auto acceptance = outputs_.enqueue(payload);

        if (acceptance != OutputEnqueueResult::accepted) {
            return acceptance;
        }

        if (usable) {
            world_.update_weather(WeatherObservation{
                *result.forecast,
                InputSource::backend,
                now_,
                WeatherLocation{
                    WeatherLocationKind::selected_location,
                    pending_weather_request_->postal_code
                }
                });
        }

        pending_weather_request_.reset();

        conversation_.expected_reply = usable
            ? ExpectedReply::none
            : ExpectedReply::swiss_postal_code;

        return OutputEnqueueResult::accepted;
    }

}