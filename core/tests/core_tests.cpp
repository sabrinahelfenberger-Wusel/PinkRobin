#include "pinkrobin/core.hpp"
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace pinkrobin;
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)

int main() {
    try {
        Core greeting;
        CHECK(greeting.snapshot().mode == Mode::sleeping);
        CHECK(greeting.advance_to(3999) == Result::ok);
        CHECK(greeting.snapshot().expression == Expression::sleeping);
        greeting.advance_to(4000);
        CHECK(greeting.snapshot().mode == Mode::waking);
        greeting.advance_to(7099);
        CHECK(greeting.snapshot().mode == Mode::waking);
        greeting.advance_to(7100);
        CHECK(greeting.snapshot().mode == Mode::awake);
        CHECK(greeting.snapshot().mood == Mood::curious);
        Core jumped;
        jumped.advance_to(1000000);
        CHECK(jumped.snapshot().mode == Mode::awake);
        CHECK(jumped.snapshot().revision == greeting.snapshot().revision);

        Core heart;
        heart.dispatch(Event::heart_touched, 500);
        CHECK(heart.snapshot().expression == Expression::happy);
        heart.advance_to(6499);
        CHECK(heart.snapshot().mood == Mood::happy);
        heart.dispatch(Event::heart_touched, 6499);
        heart.advance_to(6500);
        CHECK(heart.snapshot().mood == Mood::happy);
        heart.advance_to(12499);
        CHECK(heart.snapshot().mood == Mood::curious);
        CHECK(heart.snapshot().mode == Mode::awake);

        Core head;
        head.dispatch(Event::head_started, 0);
        head.advance_to(4999);
        CHECK(head.snapshot().expression == Expression::happy);
        const auto revision = head.snapshot().revision;
        head.dispatch(Event::head_started, 4999);
        CHECK(head.snapshot().revision == revision);
        head.advance_to(5000);
        CHECK(head.snapshot().expression == Expression::heart_eyes);
        CHECK(head.snapshot().head_contact);
        head.advance_to(100000);
        CHECK(head.snapshot().expression == Expression::heart_eyes);
        head.dispatch(Event::head_ended, 100000);
        CHECK(head.snapshot().expression == Expression::curious);
        CHECK(!head.snapshot().head_contact);
        head.dispatch(Event::head_started, 100001);
        head.dispatch(Event::interaction_cancelled, 100002);
        head.advance_to(200000);
        CHECK(head.snapshot().expression == Expression::curious);
        head.dispatch(Event::head_started, 200000);
        head.dispatch(Event::heart_touched, 200001);
        head.dispatch(Event::head_ended, 200002);
        CHECK(head.snapshot().expression == Expression::happy);
        CHECK(!head.snapshot().head_contact);
        head.advance_to(206001);
        CHECK(head.snapshot().expression == Expression::curious);

        Core reset;
        reset.dispatch(Event::head_started, 10);
        reset.dispatch(Event::restart, 1000);
        reset.advance_to(4999);
        CHECK(reset.snapshot().mode == Mode::sleeping);
        reset.advance_to(5000);
        CHECK(reset.snapshot().mode == Mode::waking);
        CHECK(!reset.snapshot().head_contact);
        reset.advance_to(8100);
        CHECK(reset.snapshot().mood == Mood::curious);
        auto before = reset.snapshot();
        CHECK(reset.advance_to(8099) == Result::time_reversed);
        CHECK(reset.dispatch(Event::heart_touched, 8099) == Result::time_reversed);
        CHECK(reset.dispatch(static_cast<Event>(999), 9000) == Result::invalid_argument);
        CHECK(reset.snapshot().time_ms == before.time_ms);
        CHECK(reset.snapshot().revision == before.revision);

        Core isolated_a, isolated_b;
        isolated_a.dispatch(Event::head_started, 0);
        isolated_a.advance_to(5000);
        CHECK(isolated_b.snapshot().mode == Mode::sleeping);
        CHECK(isolated_b.snapshot().revision == 0);
        CHECK(isolated_a.snapshot().expression == Expression::heart_eyes);
        const auto maximum = std::numeric_limits<std::uint64_t>::max();
        Core near_end(maximum - 5000);
        near_end.dispatch(Event::heart_touched, maximum - 5000);
        near_end.advance_to(maximum);
        CHECK(near_end.snapshot().mood == Mood::happy);
        CHECK(near_end.advance_to(0) == Result::time_reversed);

        Core weather_core;

        // Zu Beginn kennt Robin keine Wetterprognose.
        CHECK(!weather_core.weather().has_value());

        WeatherForecast forecast{
            8.5,    // Temperatur in °C
            12.0,   // Niederschlag in mm
            30.0,   // Wind in km/h
            70.0,   // Böen in km/h
            20000,  // Gültig ab
            30000   // Gültig bis ausschließlich
        };

        CHECK(weather_core.post_weather(WeatherUpdated{ forecast }) == Result::ok);

        // Noch nicht gültig: bleibt vorgemerkt.
        CHECK(weather_core.advance_to(15000) == Result::ok);
        CHECK(!weather_core.weather().has_value());

        // Genau am Beginn: wird übernommen.
        CHECK(weather_core.advance_to(20000) == Result::ok);

        const auto active_weather = weather_core.weather();
        CHECK(active_weather.has_value());
        CHECK(active_weather->precipitation_mm == 12.0);

        // Kurz vor dem Ende: weiterhin gültig.
        CHECK(weather_core.advance_to(29999) == Result::ok);
        CHECK(weather_core.weather().has_value());

        // Genau am Ende: nicht mehr gültig.
        CHECK(weather_core.advance_to(30000) == Result::ok);
        CHECK(!weather_core.weather().has_value());

        Core weather_validation;

        WeatherForecast valid{
            8.5, 12.0, 30.0, 70.0,
            0, 60000
        };

        CHECK(weather_validation.post_weather(
            WeatherUpdated{ valid }) == Result::ok);

        // Kopie erstellen und absichtlich ungültig machen.
        auto invalid = valid;
        invalid.precipitation_mm = -1.0;

        CHECK(weather_validation.post_weather(
            WeatherUpdated{ invalid }) == Result::invalid_argument);

        // Die ursprünglich vorgemerkte Prognose muss erhalten bleiben.
        CHECK(weather_validation.advance_to(0) == Result::ok);

        const auto retained = weather_validation.weather();
        CHECK(retained.has_value());
        CHECK(retained->precipitation_mm == 12.0);

        Core weather_replacement;

        WeatherForecast first_forecast{
            8.5, 12.0, 30.0, 70.0,
            0, 60000
        };

        auto second_forecast = first_forecast;
        second_forecast.precipitation_mm = 2.0;

        CHECK(weather_replacement.post_weather(
            WeatherUpdated{ first_forecast }) == Result::ok);

        CHECK(weather_replacement.post_weather(
            WeatherUpdated{ second_forecast }) == Result::ok);

        // Erst jetzt verarbeitet der Core die vorgemerkte Nachricht.
        CHECK(weather_replacement.advance_to(0) == Result::ok);

        const auto latest = weather_replacement.weather();
        CHECK(latest.has_value());
        CHECK(latest->precipitation_mm == 2.0);


        Core automatic_weather;

        // Robin zuerst vollständig aufwecken.
        CHECK(automatic_weather.advance_to(8000) == Result::ok);

        WeatherForecast storm_forecast{
            8.5, 12.0, 30.0, 70.0,
            8000, 3608000
        };

        // Nachricht vormerken.
        CHECK(automatic_weather.post_weather(
            WeatherUpdated{ storm_forecast }) == Result::ok);

        // Beim Verarbeiten entscheidet der Core selbst und startet Octopus.
        CHECK(automatic_weather.advance_to(8000) == Result::ok);
        CHECK(automatic_weather.snapshot().transformation ==
            Transformation::octopus);

        // Nach 9999 Millisekunden noch aktiv.
        CHECK(automatic_weather.advance_to(17999) == Result::ok);
        CHECK(automatic_weather.snapshot().transformation ==
            Transformation::octopus);

        // Nach genau zehn Sekunden beendet.
        // Der Cooldown verhindert einen sofortigen Neustart.
        CHECK(automatic_weather.advance_to(18000) == Result::ok);
        CHECK(automatic_weather.snapshot().transformation ==
            Transformation::none);


        // Der erste Start war bei 8000 ms.
        // 30 Minuten entsprechen 1800000 ms.

        // Eine Millisekunde vor Ablauf: kein neuer Start.
        CHECK(automatic_weather.advance_to(1807999) == Result::ok);
        CHECK(automatic_weather.snapshot().transformation ==
            Transformation::none);

        // Genau nach 30 Minuten: neuer Start erlaubt.
        CHECK(automatic_weather.advance_to(1808000) == Result::ok);
        CHECK(automatic_weather.snapshot().transformation ==
            Transformation::octopus);


        Core waiting_weather;

        CHECK(waiting_weather.advance_to(8000) == Result::ok);

        // Robin wird gerade gestreichelt.
        CHECK(waiting_weather.dispatch(
            Event::head_started, 8000) == Result::ok);

        WeatherForecast waiting_forecast{
            8.5, 12.0, 30.0, 70.0,
            8000, 3608000
        };

        CHECK(waiting_weather.post_weather(
            WeatherUpdated{ waiting_forecast }) == Result::ok);

        // Beschäftigt: kein Start. Nächster Versuch frühestens bei 18000.
        CHECK(waiting_weather.advance_to(8000) == Result::ok);
        CHECK(waiting_weather.snapshot().transformation ==
            Transformation::none);

        // Streicheln endet bereits nach einer Sekunde.
        CHECK(waiting_weather.dispatch(
            Event::head_ended, 9000) == Result::ok);

        // Obwohl Robin bereit ist, bleibt die Wartezeit bestehen.
        CHECK(waiting_weather.advance_to(17999) == Result::ok);
        CHECK(waiting_weather.snapshot().transformation ==
            Transformation::none);

        // Genau zehn Sekunden nach der Ablehnung startet Octopus.
        CHECK(waiting_weather.advance_to(18000) == Result::ok);
        CHECK(waiting_weather.snapshot().transformation ==
            Transformation::octopus);


        Core expired_weather;

        CHECK(expired_weather.advance_to(8000) == Result::ok);
        CHECK(expired_weather.dispatch(
            Event::head_started, 8000) == Result::ok);

        WeatherForecast short_forecast{
            8.5, 12.0, 30.0, 70.0,
            8000, 15000
        };

        CHECK(expired_weather.post_weather(
            WeatherUpdated{ short_forecast }) == Result::ok);

        // Beschäftigt: nächster Versuch frühestens bei 18000.
        CHECK(expired_weather.advance_to(8000) == Result::ok);
        CHECK(expired_weather.dispatch(
            Event::head_ended, 9000) == Result::ok);

        // Die Prognose endet bereits bei 15000.
        CHECK(expired_weather.advance_to(15000) == Result::ok);
        CHECK(!expired_weather.weather().has_value());

        // Auch beim nächsten Prüfzeitpunkt darf nichts starten.
        CHECK(expired_weather.advance_to(18000) == Result::ok);
        CHECK(expired_weather.snapshot().transformation ==
            Transformation::none);


        Core changed_weather;

        CHECK(changed_weather.advance_to(8000) == Result::ok);
        CHECK(changed_weather.dispatch(
            Event::head_started, 8000) == Result::ok);

        WeatherForecast storm{
            8.5, 12.0, 30.0, 70.0,
            8000, 60000
        };

        CHECK(changed_weather.post_weather(
            WeatherUpdated{ storm }) == Result::ok);

        // Robin ist beschäftigt und merkt sich Octopus vor.
        CHECK(changed_weather.advance_to(8000) == Result::ok);

        // Eine neue Prognose meldet ruhiges Wetter.
        auto calm = storm;
        calm.precipitation_mm = 0.0;
        calm.wind_gust_kmh = 10.0;

        CHECK(changed_weather.post_weather(
            WeatherUpdated{ calm }) == Result::ok);

        CHECK(changed_weather.dispatch(
            Event::head_ended, 9000) == Result::ok);

        // Trotz abgelaufener Wartezeit darf Octopus nicht starten.
        CHECK(changed_weather.advance_to(18000) == Result::ok);
        CHECK(changed_weather.snapshot().transformation ==
            Transformation::none);


        Core raincoat_weather;

        CHECK(raincoat_weather.advance_to(8000) == Result::ok);

        WeatherForecast rainy_forecast{
            8.5, 2.0, 10.0, 20.0,
            8000, 60000
        };

        CHECK(raincoat_weather.post_weather(
            WeatherUpdated{ rainy_forecast }) == Result::ok);

        CHECK(raincoat_weather.advance_to(8000) == Result::ok);

        CHECK(raincoat_weather.snapshot().transformation ==
            Transformation::raincoat);


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

        Core weather_message_core;

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

        Core queued_interactions;

        const Message queued_head_started{
            MessageMetadata{ InputSource::web },
            InteractionReceived{ Event::head_started }
        };

        const Message queued_head_ended{
            MessageMetadata{ InputSource::web },
            InteractionReceived{ Event::head_ended }
        };

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

        std::cout << "Core behavior, timing boundaries, interruption, isolation and clock validation passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
