#include "pinkrobin/c_api.h"
#include "pinkrobin/core.hpp"
#include <cstddef>
#include <new>

struct robin_core_handle {
    pinkrobin::Core core;
    explicit robin_core_handle(uint64_t now) noexcept : core(now) {}
};

static_assert(
    sizeof(robin_weather_v1) == 48,
    "Weather v1 must occupy 48 bytes");

static_assert(
    offsetof(robin_weather_v1, valid_from_ms) == 32,
    "Weather v1 validity offset mismatch"); 

static_assert(sizeof(robin_snapshot_v1) == 40, "ABI v1 snapshot must occupy 40 bytes");
static_assert(offsetof(robin_snapshot_v1, time_ms) == 24, "ABI v1 clock offset mismatch");


extern "C" {
    uint32_t ROBIN_CALL robin_core_abi_version(void) { return ROBIN_ABI_VERSION; }
    robin_core_handle* ROBIN_CALL robin_core_create(uint64_t now_ms) {
        return new (std::nothrow) robin_core_handle(now_ms);
    }

    void ROBIN_CALL robin_core_destroy(robin_core_handle* handle) { delete handle; }
    uint32_t ROBIN_CALL robin_core_advance(robin_core_handle* handle, uint64_t now_ms) {
        if (!handle) return ROBIN_INVALID_ARGUMENT;
        return static_cast<uint32_t>(handle->core.advance_to(now_ms));
    }

    uint32_t ROBIN_CALL robin_core_dispatch(robin_core_handle* handle, uint32_t event, uint64_t now_ms) {
        if (!handle) return ROBIN_INVALID_ARGUMENT;
        const pinkrobin::Message message{
            pinkrobin::MessageMetadata{
                pinkrobin::InputSource::web
            },
            pinkrobin::InteractionReceived{
                static_cast<pinkrobin::Event>(event)
            }
        };

        return static_cast<uint32_t>(
            handle->core.receive(message, now_ms));
    }

    uint32_t ROBIN_CALL robin_core_get_snapshot(const robin_core_handle* handle,
        robin_snapshot_v1* output, uint32_t output_size) {
        if (!handle || !output || output_size != sizeof(robin_snapshot_v1)) return ROBIN_INVALID_ARGUMENT;
        const auto state = handle->core.snapshot();
        *output = {ROBIN_ABI_VERSION, static_cast<uint32_t>(state.mode),
            static_cast<uint32_t>(state.mood), static_cast<uint32_t>(state.expression),
            state.head_contact ? 1u : 0u, 0u, state.time_ms, state.revision};
        return ROBIN_OK;
    }

    uint32_t ROBIN_CALL robin_core_start_transformation(
        robin_core_handle* handle,
        uint32_t transformation,
        uint64_t now_ms) {

        if (!handle) return ROBIN_INVALID_ARGUMENT;

        return static_cast<uint32_t>(
            handle->core.start_transformation(
                static_cast<pinkrobin::Transformation>(transformation),
                now_ms));
    }

    uint32_t ROBIN_CALL robin_core_get_transformation(
        const robin_core_handle* handle,
        uint32_t* output) {

        if (!handle || !output)
            return ROBIN_INVALID_ARGUMENT;

        *output = static_cast<uint32_t>(
            handle->core.snapshot().transformation);

        return ROBIN_OK;
    }

    uint32_t ROBIN_CALL robin_core_post_weather(
        robin_core_handle* handle,
        const robin_weather_v1* weather,
        uint32_t weather_size) {

        if (!handle || !weather ||
            weather_size != sizeof(robin_weather_v1)) {
            return ROBIN_INVALID_ARGUMENT;
        }

        const pinkrobin::WeatherForecast forecast{
            weather->temperature_c,
            weather->precipitation_mm,
            weather->wind_speed_kmh,
            weather->wind_gust_kmh,
            weather->valid_from_ms,
            weather->valid_until_ms
        };

        const pinkrobin::Message message{
            pinkrobin::MessageMetadata{
                pinkrobin::InputSource::web
            },
            pinkrobin::WeatherUpdated{ forecast }
        };

        return static_cast<uint32_t>(
            handle->core.receive(
                message,
                handle->core.snapshot().time_ms));
    }

    uint32_t ROBIN_CALL robin_core_receive_weather(
        robin_core_handle* handle,
        const robin_weather_v1* weather,
        uint32_t weather_size,
        uint32_t source,
        uint64_t now_ms) {

        if (!handle || !weather ||
            weather_size != sizeof(robin_weather_v1)) {
            return ROBIN_INVALID_ARGUMENT;
        }

        const pinkrobin::WeatherForecast forecast{
            weather->temperature_c,
            weather->precipitation_mm,
            weather->wind_speed_kmh,
            weather->wind_gust_kmh,
            weather->valid_from_ms,
            weather->valid_until_ms
        };

        const pinkrobin::Message message{
            pinkrobin::MessageMetadata{
                static_cast<pinkrobin::InputSource>(source)
            },
            pinkrobin::WeatherUpdated{ forecast }
        };

        return static_cast<uint32_t>(
            handle->core.receive(message, now_ms));
    }
}
