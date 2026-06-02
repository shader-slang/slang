//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Wave-tangled ops (WaveActiveMin/WaveActiveMax) inside a divergent `if` emit as HLSL wave intrinsics inside the respective if-arm, one per mutually-convergent thread set.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that wave ops inside a divergent `if` remain inside their respective arms
// in HLSL rather than being hoisted to the post-if merge point.
// CHECK asserts the conditional (`if(`) and both `WaveActiveMin` and `WaveActiveMax`
// appear in the emitted HLSL, confirming each op is scoped to its convergent thread set.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    if ((tid.x & 1) == 0)
    {
        result = WaveActiveMin(tid.x);
    }
    else
    {
        result = WaveActiveMax(tid.x);
    }
    output[tid.x] = result;
}

// The if-conditional is preserved in HLSL emission.
// CHECK: if(
// WaveActiveMin appears inside the then-arm.
// CHECK: WaveActiveMin
// WaveActiveMax appears inside the else-arm.
// CHECK: WaveActiveMax
