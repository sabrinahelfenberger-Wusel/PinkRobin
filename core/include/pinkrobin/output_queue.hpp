#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include "pinkrobin/output.hpp"

namespace pinkrobin {

    enum class OutputEnqueueResult : std::uint32_t {
        accepted = 0,
        full = 1,
        sequence_exhausted = 2,
        invalid_argument = 3
    };

    class OutputQueue final {
    public:
        std::optional<Output> front() const noexcept;

        OutputEnqueueResult enqueue(
            const OutputPayload& payload) noexcept;

        bool acknowledge(std::uint64_t sequence) noexcept;

    private:
        static constexpr std::size_t capacity = 8;

        std::array<Output, capacity> outputs_{};

        std::size_t head_{ 0 };
        std::size_t count_{ 0 };

        std::uint64_t last_sequence_{ 0 };
        std::uint64_t last_acknowledged_sequence_{ 0 };
    };

}