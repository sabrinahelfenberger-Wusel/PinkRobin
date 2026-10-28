#include "pinkrobin/core.hpp"
#include "pinkrobin/world_model.hpp"
#include "test_support.hpp"
#include <cstdint>

using namespace pinkrobin;

namespace {

const WeatherObservation observation{
    WeatherForecast{
        15.0, 2.0, 10.0, 20.0,
        100, 1000
    },
    InputSource::companion,
    100
};

void test_weather_storage_and_expiry() {
    WorldModel world;

    CHECK(!world.weather().has_value());

    world.update_weather(observation);

    const auto known_weather = world.weather();

    CHECK(known_weather.has_value());
    CHECK(known_weather->forecast.precipitation_mm == 2.0);
    CHECK(known_weather->source == InputSource::companion);
    CHECK(known_weather->received_at_ms == 100);

    // Kurz vor Ablauf bleibt das Wissen erhalten.
    world.expire_weather(999);
    CHECK(world.weather().has_value());

    // Genau am Ende ist es nicht mehr gültig.
    world.expire_weather(1000);
    CHECK(!world.weather().has_value());
}

void test_replacement_and_isolation() {
    WorldModel world_a;
    WorldModel world_b;

    world_a.update_weather(observation);

    auto replacement_observation = observation;
    replacement_observation.forecast.precipitation_mm = 5.0;
    replacement_observation.source = InputSource::backend;
    replacement_observation.received_at_ms = 200;

    world_a.update_weather(replacement_observation);

    const auto replaced_observation = world_a.weather();

    CHECK(replaced_observation.has_value());
    CHECK(replaced_observation->forecast.precipitation_mm == 5.0);
    CHECK(replaced_observation->source == InputSource::backend);
    CHECK(replaced_observation->received_at_ms == 200);

    // Ein anderes Modell übernimmt dieses Wissen nicht.
    CHECK(!world_b.weather().has_value());
}

void test_core_integration_and_metadata() {
    Core observed_core;

    const Message observed_weather_message{
        MessageMetadata{ InputSource::companion },
        WeatherUpdated{
            WeatherForecast{
                15.0, 2.0, 10.0, 20.0,
                0, 1000
            }
        }
    };

    CHECK(
        observed_core.receive(observed_weather_message, 100)
        == MessageAcceptance::accepted
    );

    // Annahme allein aktualisiert das World-Model noch nicht.
    CHECK(!observed_core.world().weather().has_value());

    CHECK(observed_core.advance_to(100) == Result::ok);

    const auto core_observation = observed_core.world().weather();

    CHECK(core_observation.has_value());
    CHECK(core_observation->forecast.precipitation_mm == 2.0);
    CHECK(core_observation->source == InputSource::companion);
    CHECK(core_observation->received_at_ms == 100);

    // Der Zeitfortschritt entfernt das abgelaufene Wissen.
    CHECK(observed_core.advance_to(1000) == Result::ok);
    CHECK(!observed_core.world().weather().has_value());
    CHECK(!observed_core.weather().has_value());
}

} // namespace

int main() {
    return test_support::run_tests({
        {"world_model: weather_storage_and_expiry", test_weather_storage_and_expiry},
        {"world_model: replacement_and_isolation", test_replacement_and_isolation},
        {"world_model: core_integration_and_metadata", test_core_integration_and_metadata},
    });
}
