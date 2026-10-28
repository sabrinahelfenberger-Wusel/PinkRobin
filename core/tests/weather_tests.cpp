#include "pinkrobin/core.hpp"
#include "test_support.hpp"
#include <cstdint>

using namespace pinkrobin;

namespace {

void test_forecast_validity() {
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
}

void test_forecast_validation() {
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
}

void test_forecast_replacement() {
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
}

void test_automatic_reaction_and_cooldown() {
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
}

void test_busy_retry() {
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
}

void test_expiry_while_waiting() {
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
}

void test_changed_forecast_cancels_reaction() {
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
}

void test_raincoat() {
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
}

} // namespace

int main() {
    return test_support::run_tests({
        {"weather: forecast_validity", test_forecast_validity},
        {"weather: forecast_validation", test_forecast_validation},
        {"weather: forecast_replacement", test_forecast_replacement},
        {"weather: automatic_reaction_and_cooldown", test_automatic_reaction_and_cooldown},
        {"weather: busy_retry", test_busy_retry},
        {"weather: expiry_while_waiting", test_expiry_while_waiting},
        {"weather: changed_forecast_cancels_reaction", test_changed_forecast_cancels_reaction},
        {"weather: raincoat", test_raincoat},
    });
}
