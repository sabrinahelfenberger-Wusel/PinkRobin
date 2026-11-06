#include "pinkrobin/c_api.h"
#include <stddef.h>
#include <stdio.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "Failed at line %d: %s\n", __LINE__, #c); return 1; } } while (0)
int main(void) {
    const robin_platform_capabilities_v1 text_capabilities = {
        1u, 0u
    };

    robin_snapshot_v1 state = {0};
    robin_core_handle* a = robin_core_create(100);
    robin_core_handle* b = robin_core_create(100);
    CHECK(a != NULL && b != NULL);
    CHECK(robin_core_abi_version() == 1);
    CHECK(sizeof(state) == 40 && offsetof(robin_snapshot_v1, time_ms) == 24);
    CHECK(robin_core_get_snapshot(NULL, &state, sizeof(state)) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_get_snapshot(a, NULL, sizeof(state)) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_get_snapshot(a, &state, sizeof(state) - 1) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_dispatch(NULL, ROBIN_EVENT_RESTART, 0) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_advance(NULL, 0) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_dispatch(a, 999, 200) == ROBIN_INVALID_ARGUMENT);
    CHECK(robin_core_dispatch(a, ROBIN_EVENT_HEAD_STARTED, 100) == ROBIN_OK);
    CHECK(robin_core_advance(a, 5100) == ROBIN_OK);
    CHECK(robin_core_get_snapshot(a, &state, sizeof(state)) == ROBIN_OK);
    CHECK(state.abi_version == 1 && state.reserved == 0);
    CHECK(state.expression == ROBIN_EXPRESSION_HEART_EYES && state.head_contact == 1);
    CHECK(robin_core_dispatch(a, ROBIN_EVENT_HEAD_ENDED, 5099) == ROBIN_TIME_REVERSED);
    CHECK(robin_core_dispatch(a, ROBIN_EVENT_HEAD_ENDED, 5100) == ROBIN_OK);
    CHECK(robin_core_get_snapshot(a, &state, sizeof(state)) == ROBIN_OK);
    CHECK(state.expression == ROBIN_EXPRESSION_CURIOUS && state.head_contact == 0);
    CHECK(robin_core_get_snapshot(b, &state, sizeof(state)) == ROBIN_OK);
    CHECK(state.mode == ROBIN_MODE_SLEEPING && state.revision == 0);
    robin_core_destroy(a);
    robin_core_destroy(b);
    robin_core_destroy(NULL);

    robin_core_handle* weather_receiver = robin_core_create(0);
    CHECK(weather_receiver != NULL);

    robin_weather_v1 received_weather = { 0 };
    received_weather.temperature_c = 15.0;
    received_weather.precipitation_mm = 2.0;
    received_weather.wind_speed_kmh = 10.0;
    received_weather.wind_gust_kmh = 20.0;
    received_weather.valid_from_ms = 0;
    received_weather.valid_until_ms = 60000;

    CHECK(robin_core_receive_weather(
        weather_receiver,
        &received_weather,
        sizeof(received_weather),
        ROBIN_INPUT_SOURCE_COMPANION,
        100) == ROBIN_OK);

    robin_snapshot_v1 received_snapshot = { 0 };

    CHECK(robin_core_get_snapshot(
        weather_receiver,
        &received_snapshot,
        sizeof(received_snapshot)) == ROBIN_OK);

    CHECK(received_snapshot.time_ms == 100);

    CHECK(robin_core_receive_weather(
        weather_receiver,
        &received_weather,
        sizeof(received_weather),
        999u,
        200) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_snapshot(
        weather_receiver,
        &received_snapshot,
        sizeof(received_snapshot)) == ROBIN_OK);

    CHECK(received_snapshot.time_ms == 100);

    robin_core_destroy(weather_receiver);

    robin_core_handle* interaction_receiver = robin_core_create(0);
    CHECK(interaction_receiver != NULL);

    CHECK(robin_core_receive_interaction(
        interaction_receiver,
        ROBIN_EVENT_HEART_TOUCHED,
        ROBIN_INPUT_SOURCE_WEB,
        100) == ROBIN_OK);

    robin_snapshot_v1 interaction_snapshot = { 0 };

    CHECK(robin_core_get_snapshot(
        interaction_receiver,
        &interaction_snapshot,
        sizeof(interaction_snapshot)) == ROBIN_OK);

    CHECK(interaction_snapshot.time_ms == 100);
    CHECK(interaction_snapshot.mood == ROBIN_MOOD_HAPPY);
    CHECK(interaction_snapshot.expression == ROBIN_EXPRESSION_HAPPY);

    CHECK(robin_core_receive_interaction(
        interaction_receiver,
        ROBIN_EVENT_HEAD_STARTED,
        999u,
        200) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_snapshot(
        interaction_receiver,
        &interaction_snapshot,
        sizeof(interaction_snapshot)) == ROBIN_OK);

    CHECK(interaction_snapshot.time_ms == 100);
    CHECK(interaction_snapshot.head_contact == 0u);
    CHECK(interaction_snapshot.mood == ROBIN_MOOD_HAPPY);

    const uint64_t revision_before_invalid_event =
        interaction_snapshot.revision;

    CHECK(robin_core_receive_interaction(
        interaction_receiver,
        999u,
        ROBIN_INPUT_SOURCE_WEB,
        200) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_snapshot(
        interaction_receiver,
        &interaction_snapshot,
        sizeof(interaction_snapshot)) == ROBIN_OK);

    CHECK(interaction_snapshot.time_ms == 100);
    CHECK(interaction_snapshot.revision == revision_before_invalid_event);
    CHECK(interaction_snapshot.head_contact == 0u);

    robin_core_destroy(interaction_receiver);

    robin_core_handle* output_core =
        robin_core_create_with_capabilities(
            0,
            &text_capabilities,
            sizeof(text_capabilities));
    CHECK(output_core != NULL);

    robin_output_v1 output = { 0 };

    /* Anfangs ist der Puffer leer. */
    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output)) == ROBIN_NO_OUTPUT);

    /* Ohne Wetterwissen entsteht eine unknown-Antwort. */
    CHECK(robin_core_request_weather_answer(
        output_core,
        100) == ROBIN_OK);

    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output)) == ROBIN_OK);

    CHECK(output.abi_version == ROBIN_ABI_VERSION);
    CHECK(output.kind == ROBIN_OUTPUT_WEATHER_ANSWER);
    CHECK(output.sequence == 1);
    CHECK(output.weather_answer_kind == ROBIN_WEATHER_ANSWER_UNKNOWN);
    CHECK(output.reserved == 0);

    /* Lesen entfernt nichts. */
    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output)) == ROBIN_OK);

    CHECK(output.sequence == 1);

    /* Erst die Bestätigung gibt den Platz frei. */
    CHECK(robin_core_acknowledge_output(
        output_core,
        1) == ROBIN_OK);

    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output)) == ROBIN_NO_OUTPUT);

    CHECK(output.sequence == 0);

    /* Eine Bestätigung darf wiederholt werden. */
    CHECK(robin_core_acknowledge_output(
        output_core,
        1) == ROBIN_OK);

    CHECK(robin_core_request_weather_answer(
        NULL,
        100) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_next_output(
        NULL,
        &output,
        sizeof(output)) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_next_output(
        output_core,
        NULL,
        sizeof(output)) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output) - 1) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_acknowledge_output(
        NULL,
        1) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_acknowledge_output(
        output_core,
        0) == ROBIN_INVALID_ARGUMENT);

    CHECK(robin_core_acknowledge_output(
        output_core,
        2) == ROBIN_INVALID_ARGUMENT);

    /* Die Core-Zeit steht bereits bei 100 ms. */
    CHECK(robin_core_request_weather_answer(
        output_core,
        99) == ROBIN_TIME_REVERSED);

    /* Auch die abgelehnte Anfrage darf keine Ausgabe erzeugen. */
    CHECK(robin_core_get_next_output(
        output_core,
        &output,
        sizeof(output)) == ROBIN_NO_OUTPUT);

    robin_core_destroy(output_core);

    robin_core_handle* full_output_core =
        robin_core_create_with_capabilities(
            0,
            &text_capabilities,
            sizeof(text_capabilities));
    CHECK(full_output_core != NULL);

    for (uint64_t time = 1; time <= 8; ++time) {
        CHECK(robin_core_request_weather_answer(
            full_output_core,
            time) == ROBIN_OK);
    }

    CHECK(robin_core_request_weather_answer(
        full_output_core,
        9) == ROBIN_QUEUE_FULL);

    CHECK(robin_core_acknowledge_output(
        full_output_core,
        1) == ROBIN_OK);

    /* Nach Freigabe passt wieder eine Ausgabe hinein. */
    CHECK(robin_core_request_weather_answer(
        full_output_core,
        10) == ROBIN_OK);

    robin_output_v1 queued_output = { 0 };

    for (uint64_t sequence = 2; sequence <= 9; ++sequence) {
        CHECK(robin_core_get_next_output(
            full_output_core,
            &queued_output,
            sizeof(queued_output)) == ROBIN_OK);

        CHECK(queued_output.sequence == sequence);

        CHECK(robin_core_acknowledge_output(
            full_output_core,
            sequence) == ROBIN_OK);
    }

    CHECK(robin_core_get_next_output(
        full_output_core,
        &queued_output,
        sizeof(queued_output)) == ROBIN_NO_OUTPUT);

    robin_core_destroy(full_output_core);

    CHECK(sizeof(robin_platform_capabilities_v1) == 8);

    CHECK(robin_core_create_with_capabilities(
        0,
        NULL,
        sizeof(robin_platform_capabilities_v1)) == NULL);

    const robin_platform_capabilities_v1 valid_capabilities = {
        1u, 1u
    };

    CHECK(robin_core_create_with_capabilities(
        0,
        &valid_capabilities,
        sizeof(valid_capabilities) - 1) == NULL);

    const robin_platform_capabilities_v1 invalid_text = {
        2u, 0u
    };

    CHECK(robin_core_create_with_capabilities(
        0,
        &invalid_text,
        sizeof(invalid_text)) == NULL);

    const robin_platform_capabilities_v1 invalid_outfit = {
        0u, 2u
    };

    CHECK(robin_core_create_with_capabilities(
        0,
        &invalid_outfit,
        sizeof(invalid_outfit)) == NULL);

    robin_core_handle* unsupported_core = robin_core_create(0);
    CHECK(unsupported_core != NULL);

    CHECK(robin_core_advance(
        unsupported_core,
        8000) == ROBIN_OK);

    CHECK(robin_core_start_transformation(
        unsupported_core,
        ROBIN_TRANSFORMATION_RAINCOAT,
        8000) == ROBIN_UNSUPPORTED);

    CHECK(robin_core_request_weather_answer(
        unsupported_core,
        8000) == ROBIN_UNSUPPORTED);

    robin_output_v1 unsupported_output = { 0 };

    CHECK(robin_core_get_next_output(
        unsupported_core,
        &unsupported_output,
        sizeof(unsupported_output)) == ROBIN_NO_OUTPUT);

    robin_core_destroy(unsupported_core);

    const robin_platform_capabilities_v1 conversation_capabilities = {
    1u, 0u
    };

    robin_core_handle* conversation_core =
        robin_core_create_with_capabilities(
            0,
            &conversation_capabilities,
            sizeof(conversation_capabilities));

    CHECK(conversation_core != NULL);

    CHECK(sizeof(robin_output_v2) == 40);
    CHECK(offsetof(robin_output_v2, request_id) == 24);

    const char weather_question[] = "Wie wird das Wetter?";

    CHECK(robin_core_receive_text(
        conversation_core,
        weather_question,
        (uint32_t)(sizeof(weather_question) - 1),
        ROBIN_INPUT_SOURCE_WEB,
        100) == ROBIN_OK);

    robin_output_v2 conversation_output = { 0 };

    CHECK(robin_core_get_next_output_v2(
        conversation_core,
        &conversation_output,
        sizeof(conversation_output)) == ROBIN_OK);

    CHECK(conversation_output.abi_version == ROBIN_ABI_VERSION);
    CHECK(conversation_output.kind == ROBIN_OUTPUT_CONVERSATION_REPLY);
    CHECK(conversation_output.conversation_reply_kind ==
        ROBIN_REPLY_ASK_POSTAL_CODE);
    CHECK(conversation_output.sequence == 1);
    CHECK(conversation_output.reserved == 0);

    CHECK(robin_core_acknowledge_output(
        conversation_core,
        conversation_output.sequence) == ROBIN_OK);

    const char postal_code[] = "9000";

    CHECK(robin_core_receive_text(
        conversation_core,
        postal_code,
        (uint32_t)(sizeof(postal_code) - 1),
        ROBIN_INPUT_SOURCE_WEB,
        200) == ROBIN_OK);

    CHECK(robin_core_get_next_output_v2(
        conversation_core,
        &conversation_output,
        sizeof(conversation_output)) == ROBIN_OK);

    CHECK(conversation_output.kind ==
        ROBIN_OUTPUT_WEATHER_FETCH_REQUESTED);
    CHECK(conversation_output.sequence == 2);
    CHECK(conversation_output.request_id == 1);

    CHECK(conversation_output.postal_code[0] == '9');
    CHECK(conversation_output.postal_code[1] == '0');
    CHECK(conversation_output.postal_code[2] == '0');
    CHECK(conversation_output.postal_code[3] == '0');

    const uint64_t fetch_request_id =
        conversation_output.request_id;

    CHECK(robin_core_acknowledge_output(
        conversation_core,
        conversation_output.sequence) == ROBIN_OK);

    const robin_weather_v1 conversation_weather = {
        8.5, 2.0, 10.0, 20.0,
        300, 60000
    };

    CHECK(robin_core_receive_weather_result(
        conversation_core,
        fetch_request_id,
        &conversation_weather,
        sizeof(conversation_weather),
        ROBIN_INPUT_SOURCE_BACKEND,
        300) == ROBIN_OK);

    CHECK(robin_core_get_next_output_v2(
        conversation_core,
        &conversation_output,
        sizeof(conversation_output)) == ROBIN_OK);

    CHECK(conversation_output.kind == ROBIN_OUTPUT_WEATHER_ANSWER);
    CHECK(conversation_output.weather_answer_kind ==
        ROBIN_WEATHER_ANSWER_RAIN);
    CHECK(conversation_output.sequence == 3);

    CHECK(robin_core_acknowledge_output(
        conversation_core,
        conversation_output.sequence) == ROBIN_OK);

    CHECK(robin_core_get_next_output_v2(
        conversation_core,
        &conversation_output,
        sizeof(conversation_output)) == ROBIN_NO_OUTPUT);

    CHECK(conversation_output.sequence == 0);

    robin_core_destroy(conversation_core);

    puts("Pure C consumer, ABI layout, ownership and errors passed.");
    return 0;
}
