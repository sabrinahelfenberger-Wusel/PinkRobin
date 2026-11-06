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
}

int main() {
    return test_support::run_tests({
        { "conversation: empty_context", test_empty_context },
        { "conversation: expiry_boundary", test_expiry_boundary },
        { "conversation: reversed_time", test_reversed_time },
        { "conversation: weather_location", test_weather_location_conversation },
        { "conversation: invalid_postal_code_and_expiry", test_invalid_postal_code_and_context_expiry },
        { "conversation: waits_for_output_capacity", test_text_waits_for_output_capacity },
    });
}