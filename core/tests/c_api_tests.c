#include "pinkrobin/c_api.h"
#include <stddef.h>
#include <stdio.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "Failed at line %d: %s\n", __LINE__, #c); return 1; } } while (0)
int main(void) {
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
    puts("Pure C consumer, ABI layout, ownership and errors passed.");
    return 0;
}
