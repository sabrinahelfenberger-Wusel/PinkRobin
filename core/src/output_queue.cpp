#include "pinkrobin/output_queue.hpp"
#include "pinkrobin/output_text.hpp"
#include <limits>

namespace pinkrobin {

    std::optional<Output> OutputQueue::front() const noexcept {
        if (count_ == 0) {
            return std::nullopt;
        }

        return outputs_[head_];
    }

    OutputEnqueueResult OutputQueue::enqueue(
        const OutputPayload& payload) noexcept {

        if (const auto* answer =
            std::get_if<WeatherAnswer>(&payload)) {

            if (static_cast<std::uint32_t>(answer->kind) >
                static_cast<std::uint32_t>(WeatherAnswerKind::storm)) {
                return OutputEnqueueResult::invalid_argument;
            }
        }
        else if (const auto* reply =
            std::get_if<ConversationReply>(&payload)) {

            if (static_cast<std::uint32_t>(reply->kind) >
                static_cast<std::uint32_t>(
                    ConversationReplyKind::gratitude)) {
                return OutputEnqueueResult::invalid_argument;
            }
        }
        else if (const auto* request =
            std::get_if<WeatherFetchRequested>(&payload)) {

            if (request->request_id == 0) {
                return OutputEnqueueResult::invalid_argument;
            }

            for (const auto digit : request->postal_code) {
                if (digit < '0' || digit > '9') {
                    return OutputEnqueueResult::invalid_argument;
                }
            }
        }
        else {
            return OutputEnqueueResult::invalid_argument;
        }

        if (count_ == capacity) {
            return OutputEnqueueResult::full;
        }

        if (last_sequence_ ==
            std::numeric_limits<std::uint64_t>::max()) {
            return OutputEnqueueResult::sequence_exhausted;
        }

        const auto position = (head_ + count_) % capacity;
        const auto sequence = last_sequence_ + 1;

        OutputText text{};

        if (!make_output_text(payload, text)) {
            return OutputEnqueueResult::invalid_argument;
        }

        outputs_[position] = Output{
            sequence,
            payload,
            text
        };

        last_sequence_ = sequence;
        ++count_;

        return OutputEnqueueResult::accepted;
    }

    bool OutputQueue::acknowledge(
        std::uint64_t sequence) noexcept {

        if (sequence == 0) {
            return false;
        }

        if (sequence <= last_acknowledged_sequence_) {
            return true;
        }

        if (count_ == 0 ||
            outputs_[head_].sequence != sequence) {
            return false;
        }

        last_acknowledged_sequence_ = sequence;

        head_ = (head_ + 1) % capacity;
        --count_;

        return true;
    }
}