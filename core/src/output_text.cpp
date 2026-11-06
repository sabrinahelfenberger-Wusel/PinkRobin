#include "pinkrobin/output_text.hpp"

#include <algorithm>
#include <string_view>

namespace pinkrobin {

    namespace {

        std::string_view weather_text(
            WeatherAnswerKind kind) noexcept {

            switch (kind) {
            case WeatherAnswerKind::unknown:
                return u8"Ich habe gerade keine gültigen Wetterdaten. "
                    u8"Laden wir das Wetter für deinen Ort?";

            case WeatherAnswerKind::dry:
                return u8"Die Prognose meldet trockenes Wetter. "
                    u8"Was wir wohl draussen entdecken könnten? ♡";

            case WeatherAnswerKind::rain:
                return u8"Die Prognose meldet Regen. "
                    u8"Vergiss deinen Regenschutz nicht! ♡";

            case WeatherAnswerKind::storm:
                return u8"Die Prognose meldet viel Regen und kräftige Böen. "
                    u8"Machen wir es uns lieber drinnen gemütlich? ♡";
            }

            return {};
        }

        std::string_view conversation_text(
            ConversationReplyKind kind) noexcept {

            switch (kind) {
            case ConversationReplyKind::unknown_input:
                return u8"Ich bin noch klein und verstehe noch nicht alles. "
                    u8"Aber ich lerne dazu! ♡";

            case ConversationReplyKind::ask_postal_code:
                return u8"Für welchen Ort möchtest du das Wetter wissen? "
                    u8"Nenne mir bitte eine vierstellige Schweizer PLZ. ♡";

            case ConversationReplyKind::invalid_postal_code:
                return u8"Damit kann ich noch keinen Ort bestimmen. "
                    u8"Nenne mir bitte eine vierstellige Schweizer PLZ.";

            case ConversationReplyKind::weather_unavailable:
                return u8"Für diesen Ort konnte ich gerade keine gültige "
                    u8"Prognose laden. Möchtest du eine andere "
                    u8"Schweizer PLZ versuchen?";
            }

            return {};
        }

    }

    bool make_output_text(
        const OutputPayload& payload,
        OutputText& text) noexcept {

        text = {};
        std::string_view value;

        if (const auto* answer =
            std::get_if<WeatherAnswer>(&payload)) {

            value = weather_text(answer->kind);
        }
        else if (const auto* reply =
            std::get_if<ConversationReply>(&payload)) {

            value = conversation_text(reply->kind);
        }
        else if (std::holds_alternative<WeatherFetchRequested>(
            payload)) {

            return true;
        }
        else {
            return false;
        }

        if (value.empty() ||
            value.size() > text.utf8.size()) {

            return false;
        }

        std::copy(
            value.begin(),
            value.end(),
            text.utf8.begin());

        text.length =
            static_cast<std::uint32_t>(value.size());

        return true;
    }

}