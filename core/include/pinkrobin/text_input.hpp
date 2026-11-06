#pragma once

#include <array>
#include <cstddef>

namespace pinkrobin {

    inline constexpr std::size_t text_input_capacity = 160;

    struct TextReceived {
        std::array<char, text_input_capacity> utf8{};
        std::size_t length{ 0 };
    };

    bool is_valid_text_input(const TextReceived& input) noexcept;
}