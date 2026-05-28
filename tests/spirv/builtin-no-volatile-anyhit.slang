// builtin-no-volatile-anyhit.slang
//
// Regression test for shader-slang/slang#10528.
// The Vulkan spec mandates `Volatile` on subgroup / SM builtins in
// raygeneration, closesthit, miss, intersection, and callable shaders
// only — anyhit is explicitly excluded. This test pins that exclusion
// so a future change to `isRaytracingStage` does not silently widen
// the set.
//
// All seven volatile-required subgroup-family enumerators share the
// same `isRaytracingStageRequiringVolatileBuiltins` gate, but a
// regression that drops AnyHit filtering for `SubgroupSize` or one
// of the five `Subgroup*Mask` variants (without breaking
// `SubgroupLocalInvocationId`) would slip past a single-builtin
// test. Read three independently-emitted builtins from the AnyHit
// shader: `WaveGetLaneIndex` → `SubgroupLocalInvocationId`,
// `WaveGetLaneCount` → `SubgroupSize`, and `WaveGetLaneEqMask` →
// `SubgroupEqMask`.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry anyHitMain -stage anyhit -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry anyHitMain -stage anyhit -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;
RWStructuredBuffer<uint4> output_mask;

struct Payload { uint v; };
struct Attribs { float2 b; };

// Each of the three builtin variables must appear with its `BuiltIn`
// decoration, but no `Volatile` token may appear anywhere in the
// disassembly (neither variable-level decoration nor per-access
// mask). FileCheck's `CHECK-NOT` only excludes matches in unsearched
// regions of input, so the `CHECK-DAG` block is bracketed by a
// leading `CHECK-NOT` (scans file start → first DAG match) and a
// trailing `CHECK-NOT` (scans unmatched lines between DAG anchors,
// plus the last DAG match → EOF). Together they cover the whole
// input.
// CHECK-NOT: Volatile
// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupSize
// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupEqMask
// CHECK-NOT: Volatile
[shader("anyhit")]
void anyHitMain(inout Payload p, in Attribs a)
{
    p.v = WaveGetLaneIndex();
    output[0] = WaveGetLaneCount();
    output_mask[0] = WaveGetLaneEqMask();
}
