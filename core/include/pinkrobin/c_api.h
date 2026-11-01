#ifndef PINKROBIN_C_API_H
#define PINKROBIN_C_API_H
#include <stdint.h>

#if defined(_WIN32)
# if defined(PINKROBIN_CORE_EXPORTS)
#  define ROBIN_API __declspec(dllexport)
# else
#  define ROBIN_API __declspec(dllimport)
# endif
# define ROBIN_CALL __cdecl
#else
# define ROBIN_API __attribute__((visibility("default")))
# define ROBIN_CALL
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct robin_core_handle robin_core_handle;
#define ROBIN_ABI_VERSION 1u
#define ROBIN_OK 0u
#define ROBIN_INVALID_ARGUMENT 1u
#define ROBIN_TIME_REVERSED 2u
#define ROBIN_BUSY 3u
#define ROBIN_EVENT_RESTART 0u
#define ROBIN_EVENT_HEART_TOUCHED 1u
#define ROBIN_EVENT_HEAD_STARTED 2u
#define ROBIN_EVENT_HEAD_ENDED 3u
#define ROBIN_EVENT_INTERACTION_CANCELLED 4u
#define ROBIN_MODE_SLEEPING 0u
#define ROBIN_MODE_WAKING 1u
#define ROBIN_MODE_AWAKE 2u
#define ROBIN_MOOD_CURIOUS 0u
#define ROBIN_MOOD_HAPPY 1u
#define ROBIN_MOOD_AFFECTIONATE 2u
#define ROBIN_EXPRESSION_SLEEPING 0u
#define ROBIN_EXPRESSION_CURIOUS 1u
#define ROBIN_EXPRESSION_HAPPY 2u
#define ROBIN_EXPRESSION_HEART_EYES 3u
#define ROBIN_TRANSFORMATION_NONE 0u
#define ROBIN_TRANSFORMATION_OCTOPUS 1u
#define ROBIN_TRANSFORMATION_RAINCOAT 2u
#define ROBIN_INPUT_SOURCE_WEB 0u
#define ROBIN_INPUT_SOURCE_COMPANION 1u
#define ROBIN_INPUT_SOURCE_DEVICE 2u
#define ROBIN_INPUT_SOURCE_BACKEND 3u
#define ROBIN_QUEUE_FULL 4u

#define ROBIN_NO_OUTPUT 5u
#define ROBIN_SEQUENCE_EXHAUSTED 6u

#define ROBIN_OUTPUT_WEATHER_ANSWER 1u

#define ROBIN_WEATHER_ANSWER_UNKNOWN 0u
#define ROBIN_WEATHER_ANSWER_DRY 1u
#define ROBIN_WEATHER_ANSWER_RAIN 2u
#define ROBIN_WEATHER_ANSWER_STORM 3u

#define ROBIN_UNSUPPORTED 7u

typedef struct robin_snapshot_v1 {
    uint32_t abi_version;
    uint32_t mode;
    uint32_t mood;
    uint32_t expression;
    uint32_t head_contact;
    uint32_t reserved;
    uint64_t time_ms;
    uint64_t revision;
} robin_snapshot_v1;

typedef struct robin_weather_v1 {
    double temperature_c;
    double precipitation_mm;
    double wind_speed_kmh;
    double wind_gust_kmh;
    uint64_t valid_from_ms;
    uint64_t valid_until_ms;
} robin_weather_v1;

typedef struct robin_output_v1 {
    uint32_t abi_version;
    uint32_t kind;
    uint64_t sequence;
    uint32_t weather_answer_kind;
    uint32_t reserved;
} robin_output_v1;

typedef struct robin_platform_capabilities_v1 {
    uint32_t text_output;
    uint32_t outfit_display;
} robin_platform_capabilities_v1;

ROBIN_API uint32_t ROBIN_CALL robin_core_abi_version(void);
/* NULL means allocation failed. Never share a handle across sessions. */
ROBIN_API robin_core_handle* ROBIN_CALL robin_core_create(uint64_t now_ms);
ROBIN_API robin_core_handle* ROBIN_CALL
robin_core_create_with_capabilities(
    uint64_t now_ms,
    const robin_platform_capabilities_v1* capabilities,
    uint32_t capabilities_size);
ROBIN_API void ROBIN_CALL robin_core_destroy(robin_core_handle* handle);
ROBIN_API uint32_t ROBIN_CALL robin_core_advance(robin_core_handle* handle, uint64_t now_ms);
ROBIN_API uint32_t ROBIN_CALL robin_core_dispatch(robin_core_handle* handle, uint32_t event, uint64_t now_ms);

ROBIN_API uint32_t ROBIN_CALL robin_core_receive_interaction(
    robin_core_handle* handle,
    uint32_t event,
    uint32_t source,
    uint64_t now_ms);

ROBIN_API uint32_t ROBIN_CALL robin_core_get_snapshot(const robin_core_handle* handle,
    robin_snapshot_v1* output, uint32_t output_size);
ROBIN_API uint32_t ROBIN_CALL robin_core_post_weather(
    robin_core_handle* handle,
    const robin_weather_v1* weather,
    uint32_t weather_size);
ROBIN_API uint32_t ROBIN_CALL robin_core_start_transformation(
    robin_core_handle* handle,
    uint32_t transformation,
    uint64_t now_ms);
ROBIN_API uint32_t ROBIN_CALL robin_core_get_transformation(
    const robin_core_handle* handle,
    uint32_t* output);
ROBIN_API uint32_t ROBIN_CALL robin_core_receive_weather(
    robin_core_handle* handle,
    const robin_weather_v1* weather,
    uint32_t weather_size,
    uint32_t source,
    uint64_t now_ms);

ROBIN_API uint32_t ROBIN_CALL robin_core_post_interaction(
    robin_core_handle* handle,
    uint32_t event,
    uint32_t source,
    uint64_t now_ms);

ROBIN_API uint32_t ROBIN_CALL robin_core_request_weather_answer(
    robin_core_handle* handle,
    uint64_t now_ms);

ROBIN_API uint32_t ROBIN_CALL robin_core_get_next_output(
    const robin_core_handle* handle,
    robin_output_v1* output,
    uint32_t output_size);

ROBIN_API uint32_t ROBIN_CALL robin_core_acknowledge_output(
    robin_core_handle* handle,
    uint64_t sequence);

#ifdef __cplusplus
}
#endif
#endif
