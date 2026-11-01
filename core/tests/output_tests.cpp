#include "pinkrobin/output_queue.hpp"
#include "test_support.hpp"

using namespace pinkrobin;

namespace {

    void test_enqueue_and_repeated_read() {
        OutputQueue queue;

        CHECK(!queue.front().has_value());

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::rain })
            == OutputEnqueueResult::accepted
        );

        const auto first_read = queue.front();
        CHECK(first_read.has_value());
        CHECK(first_read->sequence == 1);
        CHECK(
            std::get<WeatherAnswer>(first_read->payload).kind
            == WeatherAnswerKind::rain
        );

        const auto second_read = queue.front();
        CHECK(second_read.has_value());
        CHECK(second_read->sequence == first_read->sequence);
        CHECK(
            std::get<WeatherAnswer>(second_read->payload).kind
            == WeatherAnswerKind::rain
        );
    }

    void test_acknowledgement_and_retry() {
        OutputQueue queue;

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::rain })
            == OutputEnqueueResult::accepted
        );

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::dry })
            == OutputEnqueueResult::accepted
        );

        // Ausgabe 2 darf Ausgabe 1 nicht überholen.
        CHECK(!queue.acknowledge(2));
        CHECK(!queue.acknowledge(0));

        const auto before_ack = queue.front();
        CHECK(before_ack.has_value());
        CHECK(before_ack->sequence == 1);

        CHECK(queue.acknowledge(1));

        const auto next = queue.front();
        CHECK(next.has_value());
        CHECK(next->sequence == 2);
        CHECK(
            std::get<WeatherAnswer>(next->payload).kind
            == WeatherAnswerKind::dry
        );

        // Wiederholung entfernt nicht die nächste Ausgabe.
        CHECK(queue.acknowledge(1));

        const auto after_retry = queue.front();
        CHECK(after_retry.has_value());
        CHECK(after_retry->sequence == 2);

        CHECK(queue.acknowledge(2));
        CHECK(!queue.front().has_value());

        // Auch bei leerem Puffer ist die Bestätigung wiederholbar.
        CHECK(queue.acknowledge(2));
        CHECK(!queue.acknowledge(3));
    }
    
    void test_capacity_and_slot_reuse() {
        OutputQueue queue;

        for (int i = 0; i < 8; ++i) {
            CHECK(
                queue.enqueue(WeatherAnswer{ WeatherAnswerKind::rain })
                == OutputEnqueueResult::accepted
            );
        }

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::storm })
            == OutputEnqueueResult::full
        );

        const auto oldest = queue.front();
        CHECK(oldest.has_value());
        CHECK(oldest->sequence == 1);
        CHECK(
            std::get<WeatherAnswer>(oldest->payload).kind
            == WeatherAnswerKind::rain
        );

        // Einen Platz freigeben und am Array-Anfang wiederverwenden.
        CHECK(queue.acknowledge(1));

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::storm })
            == OutputEnqueueResult::accepted
        );

        for (std::uint64_t sequence = 2; sequence <= 9; ++sequence) {
            const auto output = queue.front();

            CHECK(output.has_value());
            CHECK(output->sequence == sequence);

            const auto expected = sequence == 9
                ? WeatherAnswerKind::storm
                : WeatherAnswerKind::rain;

            CHECK(
                std::get<WeatherAnswer>(output->payload).kind
                == expected
            );

            CHECK(queue.acknowledge(sequence));
        }

        CHECK(!queue.front().has_value());
    }

    void test_invalid_output_preserves_sequence() {
        OutputQueue queue;

        CHECK(
            queue.enqueue(WeatherAnswer{
                static_cast<WeatherAnswerKind>(999)
                })
            == OutputEnqueueResult::invalid_argument
        );

        CHECK(!queue.front().has_value());

        CHECK(
            queue.enqueue(WeatherAnswer{ WeatherAnswerKind::unknown })
            == OutputEnqueueResult::accepted
        );

        const auto output = queue.front();

        CHECK(output.has_value());
        CHECK(output->sequence == 1);
        CHECK(
            std::get<WeatherAnswer>(output->payload).kind
            == WeatherAnswerKind::unknown
        );
    }
}

int main() {
    return test_support::run_tests({
        { "output: enqueue_and_repeated_read",
            test_enqueue_and_repeated_read },
        { "output: acknowledgement_and_retry",
            test_acknowledgement_and_retry },
        { "output: capacity_and_slot_reuse",
            test_capacity_and_slot_reuse },
        { "output: invalid_output_preserves_sequence",
            test_invalid_output_preserves_sequence },
    });
}