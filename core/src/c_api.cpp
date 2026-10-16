#include "pinkrobin/c_api.h"
#include "pinkrobin/core.hpp"
#include <cstddef>
#include <new>

struct robin_core_handle {
    pinkrobin::Core core;
    explicit robin_core_handle(uint64_t now) noexcept : core(now) {}
};
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
    return static_cast<uint32_t>(handle->core.dispatch(static_cast<pinkrobin::Event>(event), now_ms));
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
}
