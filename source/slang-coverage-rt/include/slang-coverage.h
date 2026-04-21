// slang-coverage.h — Tier-1 host-side helper library for Slang shader
// execution coverage collected via `slangc -trace-coverage`.
//
// WHAT THIS DOES
//
//   The compiler's -trace-coverage flag instruments each source
//   statement with an atomic increment on a `RWStructuredBuffer<uint>
//   __slang_coverage` and emits a JSON sidecar (`.slangcov`) mapping
//   counter indices back to `(file, line)` positions.
//
//   This library encapsulates the host-side lifecycle:
//     * parse the manifest
//     * report the counter-buffer size and binding so the app can
//       allocate/bind a UAV at the right slot
//     * accumulate counter values read back after each dispatch
//     * emit an LCOV `.info` report consumable by genhtml, Codecov,
//       VS Code Coverage Gutters, etc.
//
//   The app retains ownership of GPU resources — it allocates the
//   buffer, binds it, issues the dispatch, and reads the UAV back.
//   This library is a pure CPU-side helper; it does not depend on any
//   graphics API.
//
// WHAT THIS DOES NOT DO
//
//   * No graphics-API binding. A higher-tier wrapper around slang-rhi,
//     Vulkan, or D3D12 can sit on top of this API.
//   * No lifecycle decisions — the app decides when to snapshot
//     (per frame / per test / on exit).
//
// C ABI
//
//   Everything in this header is C-callable and ABI-stable so that
//   bindings from Python, Rust, or other languages are straightforward.

#ifndef SLANG_COVERAGE_H
#define SLANG_COVERAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque handle to a coverage context. Owns the parsed manifest and
    // an accumulator over counter values seen so far.
    typedef struct SlangCoverageContext SlangCoverageContext;

    // Result codes. 0 is success; anything else is an error.
    typedef enum SlangCoverageResult
    {
        SLANG_COVERAGE_OK = 0,
        SLANG_COVERAGE_ERROR_INVALID_ARGUMENT = 1,
        SLANG_COVERAGE_ERROR_FILE_NOT_FOUND = 2,
        SLANG_COVERAGE_ERROR_PARSE_FAILED = 3,
        SLANG_COVERAGE_ERROR_UNSUPPORTED_VERSION = 4,
        SLANG_COVERAGE_ERROR_OUT_OF_RANGE = 5,
        SLANG_COVERAGE_ERROR_IO_FAILED = 6,
    } SlangCoverageResult;

    // Description of where the coverage buffer must be bound. Populated
    // from the manifest. Any field whose value is -1 was absent from the
    // manifest (e.g. CPU-target builds don't populate UAV registers).
    typedef struct SlangCoverageBindingInfo
    {
        const char* bufferName;     // e.g. "__slang_coverage"
        int32_t space;              // Vulkan descriptor set or HLSL space; -1 if absent
        int32_t binding;            // Vulkan binding index; -1 if absent
        int32_t descriptorSet;      // Alias for `space` when using Vulkan; -1 if absent
        int32_t uavRegister;        // HLSL `u<N>` register; -1 if absent
        int32_t elementStrideBytes; // typically 4
        int32_t synthesized;        // 1 if Slang synthesized this buffer, 0 if user-declared
    } SlangCoverageBindingInfo;

    // Create a context by loading a `.slangcov` manifest produced by the
    // compiler (via `SLANG_COVERAGE_MANIFEST_PATH`). On success, `*outCtx`
    // receives a handle; call `slang_coverage_destroy` when done.
    SlangCoverageResult slang_coverage_create(
        const char* manifestPath,
        SlangCoverageContext** outCtx);

    // Release a context and its resources. Safe to call on a nullptr
    // context.
    void slang_coverage_destroy(SlangCoverageContext* ctx);

    // Number of counter slots the shader will write to. Allocate a
    // `uint32_t[N]` buffer of this size for the UAV.
    uint32_t slang_coverage_counter_count(const SlangCoverageContext* ctx);

    // Reserved binding for the coverage buffer. The returned pointer is
    // valid for the lifetime of the context.
    const SlangCoverageBindingInfo* slang_coverage_binding(const SlangCoverageContext* ctx);

    // Merge one snapshot of counter values (just read back from the GPU)
    // into the context's accumulator. `counters` must point to
    // `slang_coverage_counter_count(ctx)` values. Calling this multiple
    // times with different snapshots is the typical pattern for per-frame
    // or per-test-case aggregation.
    SlangCoverageResult slang_coverage_accumulate(
        SlangCoverageContext* ctx,
        const uint32_t* counters,
        size_t count);

    // Zero the accumulator without destroying the manifest. Useful between
    // test cases when you want per-test reports.
    void slang_coverage_reset_accumulator(SlangCoverageContext* ctx);

    // Read the current accumulated hit count for a given counter index.
    // Returns 0 for out-of-range indices.
    uint64_t slang_coverage_get_hits(const SlangCoverageContext* ctx, uint32_t index);

    // Write an LCOV `.info` file for the accumulated counts. `testName`
    // is placed in the LCOV `TN:` record; LCOV forbids hyphens, so this
    // function rejects names containing `-`. Pass NULL for a default.
    SlangCoverageResult slang_coverage_save_lcov(
        const SlangCoverageContext* ctx,
        const char* outputPath,
        const char* testName);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SLANG_COVERAGE_H
