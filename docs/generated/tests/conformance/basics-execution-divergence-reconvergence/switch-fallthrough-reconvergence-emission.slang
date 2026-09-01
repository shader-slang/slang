//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Switch fall-through: threads in the first case group reconverge with threads in the second case group at the fall-through label (common body), visible as adjacent SPIRV blocks without OpSelectionMerge between them.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that fall-through between adjacent `switch` cases is handled across
// back-ends. Case 0 falls through into case 1. SPIRV and C-family targets
// (Metal, CUDA, C++) preserve the fall-through directly. WGSL does not support
// fall-through and restructures it by duplicating the shared case body.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    uint v = tid.x & 1; // 0 or 1
    switch (v)
    {
    case 0:
        result += 10;
        // fall-through — threads from case 0 reconverge with case 1 threads here
    case 1:
        result += 20;
        break;
    }
    output[tid.x] = result;
}

// SPIRV: single OpSelectionMerge for the outer switch; OpSwitch for dispatch.
// CHECK: OpSelectionMerge
// CHECK: OpSwitch

// HLSL: like WGSL, the fall-through is restructured by duplicating the shared
// case-1 body into the case-0 arm, so case 0 ends in its own break.
// CHECK_HLSL: switch(
// CHECK_HLSL: case 0U
// CHECK_HLSL: result_0 = result_0 + 10U
// CHECK_HLSL: result_0 = result_0 + 20U
// CHECK_HLSL: break
// CHECK_HLSL: case 1U
// CHECK_HLSL: result_0 = result_0 + 20U

// GLSL: fall-through preserved — the case-0 arm reaches the shared case-1 body
// with no intervening break, which is the reconvergence point in the emitted text.
// CHECK_GLSL: switch(
// CHECK_GLSL: case 0U
// CHECK_GLSL: result_0 = result_0 + 10U
// CHECK_GLSL-NOT: break
// CHECK_GLSL: case 1U
// CHECK_GLSL: result_0 = result_0 + 20U

// Metal: fall-through preserved — case 0 has no break before case 1.
// CHECK_METAL: switch(
// CHECK_METAL: case 0U
// CHECK_METAL: case 1U

// WGSL: fall-through restructured by duplicating the shared body into each arm.
// The switch statement and both case arms must still be present.
// CHECK_WGSL: switch(
// CHECK_WGSL: case u32(0)
// CHECK_WGSL: case u32(1)

// CUDA: fall-through preserved — case 0 has no break before case 1.
// CHECK_CUDA: switch(
// CHECK_CUDA: case 0U
// CHECK_CUDA: case 1U

// C++: fall-through preserved — case 0 has no break before case 1.
// CHECK_CPP: switch(
// CHECK_CPP: case 0U
// CHECK_CPP: case 1U
