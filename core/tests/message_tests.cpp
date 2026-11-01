#include "pinkrobin/core.hpp"
#include "test_support.hpp"
#include <cstdint>

using namespace pinkrobin;

namespace {

const Message valid_weather_message{
    MessageMetadata{ InputSource::companion },
    WeatherUpdated{
        WeatherForecast{
            15.0,   // Temperatur
            2.0,    // Niederschlag
            10.0,   // Wind
            20.0,   // Böen
            0,      // Gültig ab
            60000   // Gültig bis
        }
    }
};

const Message queued_head_started{
    MessageMetadata{ InputSource::web },
    InteractionReceived{ Event::head_started }
};

const Message queued_head_ended{
    MessageMetadata{ InputSource::web },
    InteractionReceived{ Event::head_ended }
};

void test_interaction_acceptance_and_source() {
    Core message_core;

    const Message heart_message{
        MessageMetadata{ InputSource::web },
        InteractionReceived{ Event::heart_touched }
    };

    CHECK(
        message_core.receive(heart_message, 100)
        == MessageAcceptance::accepted
    );

    // Angenommen, aber noch nicht ausgeführt.
    CHECK(message_core.snapshot().time_ms == 0);
    CHECK(message_core.snapshot().mode == Mode::sleeping);

    CHECK(message_core.advance_to(100) == Result::ok);

    CHECK(message_core.snapshot().time_ms == 100);
    CHECK(message_core.snapshot().mood == Mood::happy);
    CHECK(message_core.snapshot().expression == Expression::happy);

    const auto before_invalid_source = message_core.snapshot();

    const Message invalid_source_message{
        MessageMetadata{ static_cast<InputSource>(999) },
        InteractionReceived{ Event::head_started }
    };

    CHECK(
        message_core.receive(invalid_source_message, 200)
        == MessageAcceptance::invalid_argument
    );

    const auto after_invalid_source = message_core.snapshot();

    CHECK(after_invalid_source.time_ms == before_invalid_source.time_ms);
    CHECK(after_invalid_source.revision == before_invalid_source.revision);
    CHECK(after_invalid_source.head_contact == before_invalid_source.head_contact);
}

void test_expired_weather_rejected() {
    Core expired_message_core;

    const Message expired_weather_message{
        MessageMetadata{ InputSource::companion },
        WeatherUpdated{
            WeatherForecast{
                15.0,   // Temperatur
                2.0,    // Niederschlag
                10.0,   // Wind
                20.0,   // Böen
                0,      // Gültig ab
                100     // Gültig bis (exklusiv)
            }
        }
    };

    CHECK(
        expired_message_core.receive(expired_weather_message, 100)
        == MessageAcceptance::invalid_argument
    );

    CHECK(expired_message_core.snapshot().time_ms == 0);
    CHECK(expired_message_core.snapshot().revision == 0);
    CHECK(!expired_message_core.weather().has_value());
}

void test_weather_acceptance_and_reversed_time() {
    Core weather_message_core;

    CHECK(
        weather_message_core.receive(valid_weather_message, 100)
        == MessageAcceptance::accepted
    );

    // Wetter wurde nur vorgemerkt.
    CHECK(weather_message_core.snapshot().time_ms == 0);
    CHECK(!weather_message_core.weather().has_value());

    CHECK(weather_message_core.advance_to(100) == Result::ok);

    CHECK(weather_message_core.snapshot().time_ms == 100);

    const auto received_weather = weather_message_core.weather();

    CHECK(received_weather.has_value());
    CHECK(received_weather->precipitation_mm == 2.0);
    CHECK(received_weather->valid_until_ms == 60000);

    const auto before_reversed_time =
        weather_message_core.snapshot();

    CHECK(
        weather_message_core.receive(valid_weather_message, 99)
        == MessageAcceptance::time_reversed
    );

    CHECK(
        weather_message_core.snapshot().time_ms
        == before_reversed_time.time_ms
    );

    CHECK(
        weather_message_core.snapshot().revision
        == before_reversed_time.revision
    );
}

void test_interaction_order_and_timing() {
    Core queued_interactions;

    CHECK(
        queued_interactions.receive(queued_head_started, 100)
        == MessageAcceptance::accepted
    );

    CHECK(
        queued_interactions.receive(queued_head_ended, 6100)
        == MessageAcceptance::accepted
    );

    // Beide Ereignisse warten noch.
    CHECK(queued_interactions.snapshot().time_ms == 0);
    CHECK(!queued_interactions.snapshot().head_contact);

    // Nur das erste Ereignis ist jetzt fällig.
    CHECK(queued_interactions.advance_to(100) == Result::ok);
    CHECK(queued_interactions.snapshot().head_contact);

    // Fünf Sekunden nach Beginn: Herzaugen.
    CHECK(queued_interactions.advance_to(5100) == Result::ok);
    CHECK(
        queued_interactions.snapshot().expression
        == Expression::heart_eyes
    );

    // Loslassen verarbeiten.
    CHECK(queued_interactions.advance_to(6100) == Result::ok);
    CHECK(!queued_interactions.snapshot().head_contact);
    CHECK(
        queued_interactions.snapshot().expression
        == Expression::curious
    );
}

void test_capacity_and_retry() {
    Core full_queue;

    const Message repeated_head_started{
        MessageMetadata{ InputSource::web },
        InteractionReceived{ Event::head_started }
    };

    const Message release_head{
        MessageMetadata{ InputSource::web },
        InteractionReceived{ Event::head_ended }
    };

    for (std::uint64_t time = 1; time <= 8; ++time) {
        CHECK(
            full_queue.receive(repeated_head_started, time)
            == MessageAcceptance::accepted
        );
    }

    CHECK(
        full_queue.receive(release_head, 9)
        == MessageAcceptance::full
    );

    // Annahme allein hat noch nichts ausgeführt.
    CHECK(full_queue.snapshot().time_ms == 0);
    CHECK(!full_queue.snapshot().head_contact);

    // Puffer verarbeiten und dadurch Plätze freigeben.
    CHECK(full_queue.advance_to(8) == Result::ok);
    CHECK(full_queue.snapshot().head_contact);

    // Das zuvor abgelehnte Loslassen erneut übergeben.
    CHECK(
        full_queue.receive(release_head, 9)
        == MessageAcceptance::accepted
    );

    CHECK(full_queue.advance_to(9) == Result::ok);
    CHECK(!full_queue.snapshot().head_contact);
}

void test_invalid_replacement_retains_weather() {
    Core queued_weather;

    const WeatherForecast queued_forecast{
        15.0, 2.0, 10.0, 20.0,
        0, 60000
    };

    const Message initial_weather{
        MessageMetadata{ InputSource::companion },
        WeatherUpdated{ queued_forecast }
    };

    CHECK(
        queued_weather.receive(initial_weather, 100)
        == MessageAcceptance::accepted
    );

    auto invalid_forecast = queued_forecast;
    invalid_forecast.precipitation_mm = -1.0;

    const Message invalid_replacement{
        MessageMetadata{ InputSource::companion },
        WeatherUpdated{ invalid_forecast }
    };

    CHECK(
        queued_weather.receive(invalid_replacement, 200)
        == MessageAcceptance::invalid_argument
    );

    // Die Ablehnung darf auch den Empfangszeitpunkt nicht verändern.
    CHECK(queued_weather.advance_to(100) == Result::ok);

    const auto retained_weather = queued_weather.weather();

    CHECK(retained_weather.has_value());
    CHECK(retained_weather->precipitation_mm == 2.0);
    CHECK(queued_weather.snapshot().time_ms == 100);
}

void test_latest_weather_and_reception_time() {
    Core replaced_weather;

    const WeatherForecast older_forecast{
        15.0, 2.0, 10.0, 20.0,
        0, 60000
    };

    auto newer_forecast = older_forecast;
    newer_forecast.precipitation_mm = 5.0;

    CHECK(
        replaced_weather.receive(
            Message{
                MessageMetadata{ InputSource::companion },
                WeatherUpdated{ older_forecast }
            },
            100)
        == MessageAcceptance::accepted
    );

    CHECK(
        replaced_weather.receive(
            Message{
                MessageMetadata{ InputSource::companion },
                WeatherUpdated{ newer_forecast }
            },
            200)
        == MessageAcceptance::replaced
    );

    // Die alte Prognose wurde ersetzt.
    // Die neue darf vor ihrem Empfangszeitpunkt nicht erscheinen.
    CHECK(replaced_weather.advance_to(100) == Result::ok);
    CHECK(!replaced_weather.weather().has_value());

    CHECK(replaced_weather.advance_to(200) == Result::ok);

    const auto newest_weather = replaced_weather.weather();

    CHECK(newest_weather.has_value());
    CHECK(newest_weather->precipitation_mm == 5.0);
}

void test_ring_buffer_wraparound() {
    Core wrapped_queue;

    // Alle acht Plätze belegen.
    for (std::uint64_t time = 1; time <= 8; ++time) {
        CHECK(
            wrapped_queue.receive(queued_head_started, time)
            == MessageAcceptance::accepted
        );
    }

    // Drei Ereignisse entnehmen: Der Lesekopf steht jetzt bei Position 3.
    CHECK(wrapped_queue.advance_to(3) == Result::ok);
    CHECK(wrapped_queue.snapshot().head_contact);

    // Die drei freien Plätze am Array-Anfang wieder belegen.
    for (std::uint64_t time = 9; time <= 11; ++time) {
        CHECK(
            wrapped_queue.receive(queued_head_ended, time)
            == MessageAcceptance::accepted
        );
    }

    CHECK(
        wrapped_queue.receive(queued_head_ended, 12)
        == MessageAcceptance::full
    );

    // Die älteren Starts müssen vor den neu eingereihten Enden liegen.
    CHECK(wrapped_queue.advance_to(8) == Result::ok);
    CHECK(wrapped_queue.snapshot().head_contact);

    CHECK(wrapped_queue.advance_to(11) == Result::ok);
    CHECK(!wrapped_queue.snapshot().head_contact);
}

void test_ordering_across_message_types() {
    Core ordered_messages;

    CHECK(
        ordered_messages.receive(valid_weather_message, 200)
        == MessageAcceptance::accepted
    );

    // Die Core-Zeit steht noch bei 0.
    // Trotzdem darf nach einer Nachricht für 200 ms keine für 100 ms folgen.
    CHECK(
        ordered_messages.receive(queued_head_started, 100)
        == MessageAcceptance::time_reversed
    );

    // Gleicher Zeitpunkt ist erlaubt.
    CHECK(
        ordered_messages.receive(queued_head_started, 200)
        == MessageAcceptance::accepted
    );

    // Die abgelehnte Nachricht darf nicht im Puffer liegen.
    CHECK(ordered_messages.advance_to(100) == Result::ok);
    CHECK(!ordered_messages.snapshot().head_contact);

    CHECK(ordered_messages.advance_to(200) == Result::ok);
    CHECK(ordered_messages.snapshot().head_contact);
    CHECK(ordered_messages.weather().has_value());
}

} // namespace

int main() {
    return test_support::run_tests({
        {"message: interaction_acceptance_and_source", test_interaction_acceptance_and_source},
        {"message: expired_weather_rejected", test_expired_weather_rejected},
        {"message: weather_acceptance_and_reversed_time", test_weather_acceptance_and_reversed_time},
        {"message: interaction_order_and_timing", test_interaction_order_and_timing},
        {"message: capacity_and_retry", test_capacity_and_retry},
        {"message: invalid_replacement_retains_weather", test_invalid_replacement_retains_weather},
        {"message: latest_weather_and_reception_time", test_latest_weather_and_reception_time},
        {"message: ring_buffer_wraparound", test_ring_buffer_wraparound},
        {"message: ordering_across_message_types", test_ordering_across_message_types},
    });
}
