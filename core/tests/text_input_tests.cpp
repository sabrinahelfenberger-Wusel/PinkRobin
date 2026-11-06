#include "pinkrobin/text_input.hpp"
#include "test_support.hpp"
#include "pinkrobin/text_intent.hpp"

#include <string_view>
#include <initializer_list>

using namespace pinkrobin;

namespace {

    TextReceived make_text(
        std::initializer_list<unsigned char> bytes) {
        TextReceived input{};

        for (const auto byte : bytes) {
            input.utf8[input.length++] = static_cast<char>(byte);
        }

        return input;
    }

    void test_valid_text() {
        CHECK(is_valid_text_input(
            make_text({ 'H', 'a', 'l', 'l', 'o' })));

        // UTF-8: ä
        CHECK(is_valid_text_input(make_text({ 0xC3, 0xA4 })));

        // UTF-8: Herzsymbol
        CHECK(is_valid_text_input(make_text({ 0xE2, 0x99, 0xA1 })));

        // UTF-8: vier Bytes
        CHECK(is_valid_text_input(
            make_text({ 0xF0, 0x9F, 0x98, 0x80 })));
    }

    void test_empty_and_whitespace() {
        CHECK(!is_valid_text_input(TextReceived{}));
        CHECK(!is_valid_text_input(make_text({ ' ', ' ' })));

        // Geschütztes Leerzeichen
        CHECK(!is_valid_text_input(make_text({ 0xC2, 0xA0 })));
    }

    void test_length_limits() {
        TextReceived input{};
        input.utf8.fill('a');
        input.length = text_input_capacity;

        CHECK(is_valid_text_input(input));

        input.length = text_input_capacity + 1;
        CHECK(!is_valid_text_input(input));
    }

    void test_invalid_encoding_and_controls() {
        CHECK(!is_valid_text_input(make_text({ 'a', 0 })));
        CHECK(!is_valid_text_input(make_text({ '\n' })));
        CHECK(!is_valid_text_input(make_text({ 0xC2, 0x85 })));

        CHECK(!is_valid_text_input(make_text({ 0x80 })));
        CHECK(!is_valid_text_input(make_text({ 0xC3 })));
        CHECK(!is_valid_text_input(make_text({ 0xC3, 'a' })));

        // Überlange Kodierung, Surrogat und Wert oberhalb Unicode.
        CHECK(!is_valid_text_input(make_text({ 0xC0, 0xAF })));
        CHECK(!is_valid_text_input(
            make_text({ 0xED, 0xA0, 0x80 })));
        CHECK(!is_valid_text_input(
            make_text({ 0xF4, 0x90, 0x80, 0x80 })));
    }

    TextReceived make_input(std::string_view text) {
        TextReceived input{};

        if (text.size() > input.utf8.size()) {
            input.length = text.size();
            return input;
        }

        for (const auto byte : text) {
            input.utf8[input.length++] = byte;
        }

        return input;
    }

    void test_weather_intents() {
        for (const auto text : {
            "Wie ist das Wetter?",
            "wie wird das wetter",
            "  WIE   WIRD DAS WETTER?!  "
            }) {
            CHECK(recognize_text(
                make_input(text),
                ExpectedReply::none).kind ==
                TextIntentKind::weather_question);
        }

        CHECK(recognize_text(
            make_input("Hallo Robin"),
            ExpectedReply::none).kind ==
            TextIntentKind::unknown);

        // Gueltiges UTF-8 bleibt unbekannt, statt falsch erkannt zu werden.
        CHECK(recognize_text(
            make_text({ 0xC3, 0xA4 }),
            ExpectedReply::none).kind ==
            TextIntentKind::unknown);
    }

    void test_postal_code_intents() {
        const auto input = make_input(" 9000 ");

        CHECK(recognize_text(
            input,
            ExpectedReply::none).kind ==
            TextIntentKind::unknown);

        const auto recognized = recognize_text(
            input,
            ExpectedReply::swiss_postal_code);

        CHECK(recognized.kind == TextIntentKind::swiss_postal_code);
        CHECK(recognized.postal_code[0] == '9');
        CHECK(recognized.postal_code[1] == '0');
        CHECK(recognized.postal_code[2] == '0');
        CHECK(recognized.postal_code[3] == '0');

        for (const auto text : { "900", "90000", "9a00", "" }) {
            CHECK(recognize_text(
                make_input(text),
                ExpectedReply::swiss_postal_code).kind ==
                TextIntentKind::unknown);
        }
    }
}

int main() {
    return test_support::run_tests({
        { "text: valid", test_valid_text },
        { "text: empty_and_whitespace", test_empty_and_whitespace },
        { "text: length_limits", test_length_limits },
        { "text: invalid_encoding", test_invalid_encoding_and_controls },
        { "text: weather_intents", test_weather_intents },
        { "text: postal_code_intents", test_postal_code_intents },
    });
}