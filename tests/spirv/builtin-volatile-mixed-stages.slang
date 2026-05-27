// builtin-volatile-mixed-stages.slang
//
// Regression test for shader-slang/slang#10528.
// A single Slang module with both `cs()` (compute) and `rg()`
// (raygeneration) entry points reading `WaveGetLaneIndex()`. Pins the
// observed end-to-end SPIR-V emit behaviour so that a regression in
// either the IR-side referencing-entry-points propagation or the
// `BuiltinSpvVarKey` keying would be visible:
//
// With the current IR pipeline, the `SPIRVAsmOperandBuiltinVar` is
// shared / referenced from both entry points. Because at least one
// referencing stage (`raygeneration`) is in the raytracing-volatile
// set, `needVolatileDecorationForBuiltinVar` returns true and the
// single emitted `OpVariable` is decorated `Volatile` under the
// GLSL450 memory model. The `OpEntryPoint` for the compute entry
// point therefore inherits `Volatile` semantics on its load —
// overconservative but spec-correct. `BuiltinSpvVarKey` is keyed on
// (builtin, storage class, flat, pointee type); if a future IR pass
// kept distinct builtin operands per call-site, this test would need
// to assert two `OpVariable`s instead of one.
//
// What this test pins, regardless of which side of that split the
// pipeline lands on:
// - the `BuiltIn SubgroupLocalInvocationId` decoration is present;
// - the `Volatile` decoration is present (not lost by an over-eager
//   compute-only filter);
// - both entry points compile cleanly and reference a builtin in
//   their `OpEntryPoint` interface list.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry cs -entry rg -emit-spirv-directly

RWStructuredBuffer<uint> output;

// Both entry points are emitted and each lists `SubgroupLocalInvocationId`
// in its `OpEntryPoint` interface.
// CHECK: OpEntryPoint GLCompute %{{[A-Za-z0-9_]+}} "cs" {{.*}}SubgroupLocalInvocationId
// CHECK: OpEntryPoint RayGenerationKHR %{{[A-Za-z0-9_]+}} "rg" {{.*}}SubgroupLocalInvocationId

// The builtin variable is decorated `BuiltIn` and `Volatile`. The
// raygen path requires the `Volatile` decoration; if the cache axis
// or stage-set logic were to drop it, this would fail.
// CHECK-DAG: OpDecorate %[[V:[A-Za-z0-9_]+]] BuiltIn SubgroupLocalInvocationId
// CHECK-DAG: OpDecorate %[[V]] Volatile

[numthreads(64, 1, 1)]
[shader("compute")]
void cs()
{
    output[0] = WaveGetLaneIndex();
}

[shader("raygeneration")]
void rg()
{
    output[1] = WaveGetLaneIndex();
}
