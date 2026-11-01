#include "pinkrobin/output_queue.hpp"
#include "test_support.hpp"
#include "pinkrobin/core.hpp"

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

    void test_core_weather_answer_and_acknowledgement() {
        Core core{ 0, PlatformCapabilities{ true, true } };

        CHECK(!core.next_output().has_value());

        const auto before = core.snapshot();

        CHECK(
            core.request_weather_answer()
            == OutputEnqueueResult::accepted
        );

        const auto output = core.next_output();

        CHECK(output.has_value());
        CHECK(output->sequence == 1);
        CHECK(
            std::get<WeatherAnswer>(output->payload).kind
            == WeatherAnswerKind::unknown
        );

        // Lesen entfernt die Ausgabe nicht.
        const auto repeated_read = core.next_output();
        CHECK(repeated_read.has_value());
        CHECK(repeated_read->sequence == output->sequence);

        // Die Frage verändert Robins Zustand und Zeit nicht.
        const auto after = core.snapshot();
        CHECK(after.mode == before.mode);
        CHECK(after.mood == before.mood);
        CHECK(after.time_ms == before.time_ms);
        CHECK(after.revision == before.revision);

        CHECK(core.acknowledge_output(output->sequence));
        CHECK(!core.next_output().has_value());
    }

    void test_weather_answer_while_busy_and_on_cooldown() {
        Core core{ 0, PlatformCapabilities{ true, true } };

        CHECK(core.advance_to(8000) == Result::ok);

        const Message forecast{
            MessageMetadata{ InputSource::companion },
            WeatherUpdated{
                WeatherForecast{
                    8.5, 12.0, 30.0, 70.0,
                    8000, 3608000
                }
            }
        };

        CHECK(
            core.receive(forecast, 8000)
            == MessageAcceptance::accepted
        );

        CHECK(core.advance_to(8000) == Result::ok);
        CHECK(core.state().transformation == Transformation::octopus);

        // Die Verwandlung endet; ihr Cooldown läuft weiter.
        CHECK(core.advance_to(18000) == Result::ok);
        CHECK(core.state().transformation == Transformation::none);

        CHECK(
            core.dispatch(Event::head_started, 18000)
            == Result::ok
        );

        const auto before = core.snapshot();

        CHECK(
            core.request_weather_answer()
            == OutputEnqueueResult::accepted
        );

        const auto output = core.next_output();
        CHECK(output.has_value());
        CHECK(
            std::get<WeatherAnswer>(output->payload).kind
            == WeatherAnswerKind::storm
        );

        const auto after = core.snapshot();
        CHECK(after.head_contact);
        CHECK(after.mood == before.mood);
        CHECK(after.transformation == before.transformation);
        CHECK(after.time_ms == before.time_ms);
        CHECK(after.revision == before.revision);
    }

    void test_weather_answer_kinds_and_expiry() {
        struct AnswerCase {
            double precipitation_mm;
            double wind_gust_kmh;
            WeatherAnswerKind expected;
        };

        for (const auto& test : {
            AnswerCase{ 0.0, 20.0, WeatherAnswerKind::dry },
            AnswerCase{ 0.1, 20.0, WeatherAnswerKind::rain },
            AnswerCase{ 10.0, 59.0, WeatherAnswerKind::rain },
            AnswerCase{ 10.0, 60.0, WeatherAnswerKind::storm }
            }) {
            Core core{ 0, PlatformCapabilities{ true, true } };

            const Message forecast{
                MessageMetadata{ InputSource::companion },
                WeatherUpdated{
                    WeatherForecast{
                        15.0,
                        test.precipitation_mm,
                        10.0,
                        test.wind_gust_kmh,
                        0,
                        1000
                    }
                }
            };

            CHECK(
                core.receive(forecast, 100)
                == MessageAcceptance::accepted
            );

            CHECK(core.advance_to(100) == Result::ok);
            CHECK(
                core.request_weather_answer()
                == OutputEnqueueResult::accepted
            );

            const auto answer = core.next_output();
            CHECK(answer.has_value());
            CHECK(
                std::get<WeatherAnswer>(answer->payload).kind
                == test.expected
            );

            CHECK(core.acknowledge_output(answer->sequence));

            // Nach Ablauf darf die alte Prognose nicht beantwortet werden.
            CHECK(core.advance_to(1000) == Result::ok);
            CHECK(
                core.request_weather_answer()
                == OutputEnqueueResult::accepted
            );

            const auto expired_answer = core.next_output();
            CHECK(expired_answer.has_value());
            CHECK(
                std::get<WeatherAnswer>(expired_answer->payload).kind
                == WeatherAnswerKind::unknown
            );
        }
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
        { "output: core_weather_answer_and_acknowledgement",
            test_core_weather_answer_and_acknowledgement },
        { "output: weather_answer_while_busy_and_on_cooldown",
            test_weather_answer_while_busy_and_on_cooldown },
        { "output: weather_answer_kinds_and_expiry",
            test_weather_answer_kinds_and_expiry },

    });
}