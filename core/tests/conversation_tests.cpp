#include "pinkrobin/conversation_state.hpp"
#include "test_support.hpp"
#include "pinkrobin/core.hpp"
#include <string_view>

using namespace pinkrobin;

namespace {

    void test_empty_context() {
        ConversationState state;

        CHECK(!state.expire(120000));
        CHECK(state.topic == ConversationTopic::none);
        CHECK(state.expected_reply == ExpectedReply::none);
        CHECK(!state.last_input_at_ms.has_value());
    }

    void test_expiry_boundary() {
        ConversationState state{
            ConversationTopic::weather,
            ExpectedReply::swiss_postal_code,
            8000
        };

        CHECK(!state.expire(127999));
        CHECK(state.expected_reply ==
            ExpectedReply::swiss_postal_code);
        CHECK(state.last_input_at_ms == 8000);

        CHECK(state.expire(128000));
        CHECK(state.topic == ConversationTopic::none);
        CHECK(state.expected_reply == ExpectedReply::none);
        CHECK(!state.last_input_at_ms.has_value());

        CHECK(!state.expire(128001));
    }

    void test_reversed_time() {
        ConversationState state{
            ConversationTopic::weather,
            ExpectedReply::swiss_postal_code,
            8000
        };

        CHECK(!state.expire(7999));
        CHECK(state.topic == ConversationTopic::weather);
        CHECK(state.expected_reply ==
            ExpectedReply::swiss_postal_code);
        CHECK(state.last_input_at_ms == 8000);
    }


    Message text_message(std::string_view value) {
        TextReceived text{};

        CHECK(value.size() <= text.utf8.size());

        for (const auto byte : value) {
            text.utf8[text.length++] = byte;
        }

        return Message{
            MessageMetadata{ InputSource::web },
            text
        };
    }

    void test_weather_location_conversation() {
        Core core{
            0, PlatformCapabilities{ true, false }
        };

        CHECK(core.receive(
            text_message("Wie wird das Wetter?"),
            100) == MessageAcceptance::accepted);

        // Annahme allein verarbeitet die Eingabe noch nicht.
        CHECK(!core.next_output().has_value());

        CHECK(core.advance_to(100) == Result::ok);

        const auto question = core.next_output();

        CHECK(question.has_value());
        CHECK(std::get<ConversationReply>(question->payload).kind ==
            ConversationReplyKind::ask_postal_code);

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::swiss_postal_code);

        CHECK(core.acknowledge_output(question->sequence));

        CHECK(core.receive(
            text_message("9000"),
            200) == MessageAcceptance::accepted);

        CHECK(core.advance_to(200) == Result::ok);

        const auto request = core.next_output();

        CHECK(request.has_value());

        const auto& fetch =
            std::get<WeatherFetchRequested>(request->payload);

        CHECK(fetch.request_id == 1);
        CHECK(fetch.postal_code[0] == '9');
        CHECK(fetch.postal_code[1] == '0');
        CHECK(fetch.postal_code[2] == '0');
        CHECK(fetch.postal_code[3] == '0');

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::none);

        CHECK(core.acknowledge_output(request->sequence));

        const WeatherForecast forecast{
            8.5, 2.0, 10.0, 20.0,
            300, 60000
        };

        const Message weather_result{
            MessageMetadata{ InputSource::backend },
            WeatherFetchResult{ fetch.request_id, forecast }
        };

        CHECK(core.receive(
            weather_result,
            300) == MessageAcceptance::accepted);

        CHECK(core.advance_to(300) == Result::ok);

        const auto answer = core.next_output();

        CHECK(answer.has_value());
        CHECK(std::get<WeatherAnswer>(answer->payload).kind ==
            WeatherAnswerKind::rain);

        const auto observation = core.world().weather();

        CHECK(observation.has_value());
        CHECK(observation->location.kind ==
            WeatherLocationKind::selected_location);
        CHECK(observation->location.postal_code == fetch.postal_code);

        CHECK(core.acknowledge_output(answer->sequence));

        // Der Ort ist jetzt ausdrücklich gewählt:
        // Eine erneute Wetterfrage braucht keine Ortsrückfrage.
        CHECK(core.receive(
            text_message("Wie ist das Wetter?"),
            400) == MessageAcceptance::accepted);

        CHECK(core.advance_to(400) == Result::ok);

        const auto repeated_answer = core.next_output();

        CHECK(repeated_answer.has_value());
        CHECK(std::holds_alternative<WeatherAnswer>(
            repeated_answer->payload));

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::none);
    }

    void test_invalid_postal_code_and_context_expiry() {
        Core core{
            0, PlatformCapabilities{ true, false }
        };

        CHECK(core.receive(
            text_message("Wie ist das Wetter?"),
            100) == MessageAcceptance::accepted);

        CHECK(core.advance_to(100) == Result::ok);
        CHECK(core.acknowledge_output(core.next_output()->sequence));

        CHECK(core.receive(
            text_message("St. Gallen"),
            200) == MessageAcceptance::accepted);

        CHECK(core.advance_to(200) == Result::ok);

        const auto invalid = core.next_output();

        CHECK(invalid.has_value());
        CHECK(std::get<ConversationReply>(invalid->payload).kind ==
            ConversationReplyKind::invalid_postal_code);

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::swiss_postal_code);

        CHECK(core.acknowledge_output(invalid->sequence));

        // Polling verlängert die Gesprächsfrist nicht.
        CHECK(core.advance_to(120199) == Result::ok);
        CHECK(core.conversation().expected_reply ==
            ExpectedReply::swiss_postal_code);

        CHECK(core.advance_to(120200) == Result::ok);
        CHECK(core.conversation().expected_reply ==
            ExpectedReply::none);

        // Nach Ablauf ist eine einzelne PLZ keine Ortsantwort mehr.
        CHECK(core.receive(
            text_message("9000"),
            120200) == MessageAcceptance::accepted);

        CHECK(core.advance_to(120200) == Result::ok);

        const auto unknown = core.next_output();

        CHECK(unknown.has_value());
        CHECK(std::get<ConversationReply>(unknown->payload).kind ==
            ConversationReplyKind::unknown_input);
    }

    void test_text_waits_for_output_capacity() {
        Core core{
            0, PlatformCapabilities{ true, false }
        };

        for (int i = 0; i < 8; ++i) {
            CHECK(core.request_weather_answer() ==
                OutputEnqueueResult::accepted);
        }

        CHECK(core.receive(
            text_message("Wie wird das Wetter?"),
            100) == MessageAcceptance::accepted);

        CHECK(core.advance_to(100) == Result::ok);

        // Ohne Platz für die Rückfrage bleibt der Kontext unverändert.
        CHECK(core.conversation().expected_reply ==
            ExpectedReply::none);

        // Zeit läuft weiter, obwohl die Texteingabe wartet.
        CHECK(core.advance_to(200) == Result::ok);
        CHECK(core.snapshot().time_ms == 200);

        CHECK(core.acknowledge_output(1));
        CHECK(core.advance_to(200) == Result::ok);

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::swiss_postal_code);

        // Die neue Rückfrage steht hinter den vorhandenen Ausgaben.
        for (std::uint64_t sequence = 2; sequence <= 8; ++sequence) {
            CHECK(core.acknowledge_output(sequence));
        }

        const auto question = core.next_output();

        CHECK(question.has_value());
        CHECK(question->sequence == 9);
        CHECK(std::get<ConversationReply>(question->payload).kind ==
            ConversationReplyKind::ask_postal_code);

        CHECK(core.acknowledge_output(9));
        CHECK(core.advance_to(200) == Result::ok);
        CHECK(!core.next_output().has_value());
    }

    void test_weather_fetch_failure() {
        Core core{
            0, PlatformCapabilities{ true, false }
        };

        CHECK(core.receive(
            text_message("Wie wird das Wetter?"),
            100) == MessageAcceptance::accepted);
        CHECK(core.advance_to(100) == Result::ok);
        CHECK(core.acknowledge_output(core.next_output()->sequence));

        CHECK(core.receive(
            text_message("9000"),
            200) == MessageAcceptance::accepted);
        CHECK(core.advance_to(200) == Result::ok);

        const auto request = core.next_output();
        CHECK(request.has_value());

        const auto request_id =
            std::get<WeatherFetchRequested>(
                request->payload).request_id;

        CHECK(core.acknowledge_output(request->sequence));

        const Message failure{
            MessageMetadata{ InputSource::backend },
            WeatherFetchResult{ request_id, std::nullopt }
        };

        CHECK(core.receive(
            failure, 300) == MessageAcceptance::accepted);
        CHECK(core.advance_to(300) == Result::ok);

        const auto reply = core.next_output();
        CHECK(reply.has_value());

        CHECK(std::get<ConversationReply>(reply->payload).kind ==
            ConversationReplyKind::weather_unavailable);

        CHECK(core.conversation().expected_reply ==
            ExpectedReply::swiss_postal_code);

        CHECK(!core.world().weather().has_value());
        CHECK(core.acknowledge_output(reply->sequence));

        // Eine wiederholte Rückmeldung erzeugt keine zweite Antwort.
        CHECK(core.receive(
            failure, 400) == MessageAcceptance::accepted);
        CHECK(core.advance_to(400) == Result::ok);
        CHECK(!core.next_output().has_value());

        // Nur Nutzereingaben verlängern die Gesprächsfrist.
        CHECK(core.conversation().last_input_at_ms == 200);

        // Nach dem Ladefehler darf der Nutzer eine neue PLZ eingeben.
        CHECK(core.receive(
            text_message("8001"),
            500) == MessageAcceptance::accepted);

        CHECK(core.advance_to(500) == Result::ok);

        const auto second_request = core.next_output();
        CHECK(second_request.has_value());

        const auto second_request_id =
            std::get<WeatherFetchRequested>(
                second_request->payload).request_id;

        CHECK(second_request_id == request_id + 1);
        CHECK(core.acknowledge_output(second_request->sequence));

        // Das Gespräch läuft ab, bevor die Wetterdaten eintreffen.
        CHECK(core.advance_to(120500) == Result::ok);
        CHECK(core.conversation().topic == ConversationTopic::none);

        const WeatherForecast late_forecast{
            8.5, 2.0, 10.0, 20.0,
            120500, 180000
        };

        const Message late_result{
            MessageMetadata{ InputSource::backend },
            WeatherFetchResult{ second_request_id, late_forecast }
        };

        CHECK(core.receive(
            late_result,
            120501) == MessageAcceptance::accepted);

        CHECK(core.advance_to(120501) == Result::ok);

        CHECK(!core.next_output().has_value());
        CHECK(!core.world().weather().has_value());
        CHECK(core.conversation().topic == ConversationTopic::none);
    }
}

int main() {
    return test_support::run_tests({
        { "conversation: empty_context", test_empty_context },
        { "conversation: expiry_boundary", test_expiry_boundary },
        { "conversation: reversed_time", test_reversed_time },
        { "conversation: weather_location", test_weather_location_conversation },
        { "conversation: invalid_postal_code_and_expiry", test_invalid_postal_code_and_context_expiry },
        { "conversation: waits_for_output_capacity", test_text_waits_for_output_capacity },
        { "conversation: weather_fetch_failure", test_weather_fetch_failure },
    });
}