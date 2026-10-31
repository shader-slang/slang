// Compiled directly by nvcc in ci-slang-test.yml; no slang-test directive on
// purpose (slang-test's CUDA path uses NVRTC, and the point of this fixture is
// to compile the `sured` surface-reduction helpers with the *offline* toolchain
// through to SASS — which requires ptxas, not NVRTC's PTX-only path).
//
// Compiling is the whole test (shader-slang/slang#12636): each
// `__slang_surface_reduce_<op>_<ctype>` helper embeds raw `sured.b` PTX, so a
// helper whose (op, ctype, geometry) triple ptxas rejects — or a value argument
// that does not match the helper's parameter type — is a compile error here.
// This guards two regressions the SIMPLE emit test cannot catch, because it only
// checks the emitted CUDA/PTX *text*:
//   * that ptxas actually accepts every (op, ctype) combination we lower to
//     (in particular the signed and 64-bit min/max forms), and
//   * that a 64-bit literal (`unsigned long long`) is not an ambiguous overload
//     against the u64/s64 helpers — the reason the helpers are named per-ctype
//     rather than overloaded on the value type.

#include "slang-cuda-prelude.h"

static_assert(
    !SLANG_CUDA_RTC,
    "this fixture must be compiled with offline nvcc (ptxas), not NVRTC; the "
    "point is to assemble the raw sured PTX all the way to SASS");

__global__ void testSuredHelpers(cudaSurfaceObject_t s)
{
    // add: sign-agnostic, u32 and u64. Pass the exact literal widths the emitter
    // produces (`4U`, `4ULL`) to also pin down that the 64-bit call is
    // unambiguous against the per-ctype helper names.
    __slang_surface_reduce_add_u32(s, 4, 5U);
    __slang_surface_reduce_add_u32(s, 4, 8, 5U);
    __slang_surface_reduce_add_u32(s, 4, 8, 12, 5U);
    __slang_surface_reduce_add_u64(s, 8, 5ULL);
    __slang_surface_reduce_add_u64(s, 8, 8, 5ULL);
    __slang_surface_reduce_add_u64(s, 8, 8, 16, 5ULL);

    // min / max: signed and unsigned, 32- and 64-bit.
    __slang_surface_reduce_min_u32(s, 4, 8, 5U);
    __slang_surface_reduce_min_s32(s, 4, 8, (int)-5);
    __slang_surface_reduce_min_u64(s, 8, 8, 5ULL);
    __slang_surface_reduce_min_s64(s, 8, 8, (long long)-5);
    __slang_surface_reduce_max_u32(s, 4, 8, 5U);
    __slang_surface_reduce_max_s32(s, 4, 8, (int)-5);
    __slang_surface_reduce_max_u64(s, 8, 8, 5ULL);
    __slang_surface_reduce_max_s64(s, 8, 8, (long long)-5);

    // and / or: bitwise b32 only.
    __slang_surface_reduce_and_b32(s, 4, 8, 0xFU);
    __slang_surface_reduce_or_b32(s, 4, 8, 0x10U);
}
