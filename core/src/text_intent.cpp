#include "pinkrobin/text_intent.hpp"

#include <array>
#include <string_view>

namespace pinkrobin {

    TextIntent recognize_text(
        const TextReceived& input,
        ExpectedReply expected_reply) noexcept {

        if (!is_valid_text_input(input)) {
            return {};
        }

        std::array<char, text_input_capacity> normalized{};
        std::size_t length = 0;
        bool pending_space = false;

        for (std::size_t i = 0; i < input.length; ++i) {
            const auto byte =
                static_cast<unsigned char>(input.utf8[i]);

            // Für die ersten bekannten Sätze genügt ASCII.
            // Andere gültige UTF-8-Texte bleiben unbekannt.
            if (byte > 0x7F) {
                return {};
            }

            if (byte == ' ') {
                if (length > 0) {
                    pending_space = true;
                }
                continue;
            }

            if (pending_space) {
                normalized[length++] = ' ';
                pending_space = false;
            }

            normalized[length++] =
                byte >= 'A' && byte <= 'Z'
                ? static_cast<char>(byte + ('a' - 'A'))
                : static_cast<char>(byte);
        }

        // Abschliessende Satzzeichen und Leerzeichen ignorieren.
        while (length > 0) {
            const auto last = normalized[length - 1];

            if (last != '?' && last != '!' &&
                last != '.' && last != ' ') {
                break;
            }

            --length;
        }

        const std::string_view text{
            normalized.data(), length
        };

        if (text == "wie ist das wetter" ||
            text == "wie wird das wetter") {
            return {
                TextIntentKind::weather_question,
                {}
            };
        }

        if (expected_reply == ExpectedReply::swiss_postal_code &&
            text.size() == 4) {
            TextIntent result{
                TextIntentKind::swiss_postal_code,
                {}
            };

            for (std::size_t i = 0; i < 4; ++i) {
                if (text[i] < '0' || text[i] > '9') {
                    return {};
                }

                result.postal_code[i] = text[i];
            }

            return result;
        }

        return {};
    }

}