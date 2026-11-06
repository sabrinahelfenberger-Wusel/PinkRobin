#include "pinkrobin/text_input.hpp"

#include <cstdint>

namespace pinkrobin {

    namespace {

        bool is_space(std::uint32_t codepoint) noexcept {
            return codepoint == 0x20 ||
                codepoint == 0xA0 ||
                codepoint == 0x1680 ||
                (codepoint >= 0x2000 && codepoint <= 0x200A) ||
                codepoint == 0x2028 ||
                codepoint == 0x2029 ||
                codepoint == 0x202F ||
                codepoint == 0x205F ||
                codepoint == 0x3000;
        }

    }

    bool is_valid_text_input(const TextReceived& input) noexcept {
        if (input.length == 0 ||
            input.length > input.utf8.size()) {
            return false;
        }

        bool has_content = false;
        std::size_t position = 0;

        while (position < input.length) {
            const auto first =
                static_cast<unsigned char>(input.utf8[position++]);

            std::uint32_t codepoint;
            std::uint32_t minimum;
            std::size_t continuation_count;

            if (first <= 0x7F) {
                codepoint = first;
                minimum = 0;
                continuation_count = 0;
            }
            else if (first >= 0xC2 && first <= 0xDF) {
                codepoint = first & 0x1Fu;
                minimum = 0x80;
                continuation_count = 1;
            }
            else if (first >= 0xE0 && first <= 0xEF) {
                codepoint = first & 0x0Fu;
                minimum = 0x800;
                continuation_count = 2;
            }
            else if (first >= 0xF0 && first <= 0xF4) {
                codepoint = first & 0x07u;
                minimum = 0x10000;
                continuation_count = 3;
            }
            else {
                return false;
            }

            if (continuation_count > input.length - position) {
                return false;
            }

            for (std::size_t i = 0; i < continuation_count; ++i) {
                const auto next =
                    static_cast<unsigned char>(input.utf8[position++]);

                if ((next & 0xC0u) != 0x80u) {
                    return false;
                }

                codepoint = (codepoint << 6) | (next & 0x3Fu);
            }

            if (codepoint < minimum ||
                codepoint > 0x10FFFF ||
                (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                return false;
            }

            // Steuerzeichen einschliesslich Nullbyte abweisen.
            if (codepoint < 0x20 ||
                (codepoint >= 0x7F && codepoint <= 0x9F)) {
                return false;
            }

            if (!is_space(codepoint)) {
                has_content = true;
            }
        }

        return has_content;
    }

}