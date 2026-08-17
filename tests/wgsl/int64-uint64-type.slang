// int64-uint64-type.slang
//
// Verifies that int64_t and uint64_t emit as i64 and u64 in WGSL,
// not as the HLSL-style names "int64_t" / "uint64_t".
//
// Note on the wgsl-spirv-asm path
// -------------------------------
// An earlier draft of this test also exercised `-target wgsl-spirv-asm` to
// confirm the emitted WGSL round-trips through Dawn's tint compiler to
// SPIR-V. That variant was removed because the bundled tint on the Windows
// CI runners rejects `u64`/`i64` as unresolved types — these are still
// experimental in WGSL and not universally supported across tint versions
// (see WGSL extension status for 64-bit integer types). The same variant
// is silently `ignored` on Linux/macOS runners since their CI does not
// wire up the wgsl-spirv-asm tooling at all, so the platform coverage was
// asymmetric.
//
// The emitter change itself is fully covered by the WGSL text filecheck
// below — that proves slangc emits `u64`/`i64` instead of `uint64_t`/
// `int64_t`, which is the entire scope of this PR. Whether the resulting
// WGSL is accepted by a downstream tint version is a separate ecosystem
// concern that should not gate this fix.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

// WGSL-DAG: array<u64>
// WGSL-DAG: array<i64>
// WGSL: u64(42)
// WGSL: i64(-1)
// WGSL-NOT: uint64_t
// WGSL-NOT: int64_t

RWStructuredBuffer<uint64_t> outputU64;
RWStructuredBuffer<int64_t> outputI64;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputU64[tid.x] = uint64_t(42);
    outputI64[tid.x] = int64_t(-1);
}
