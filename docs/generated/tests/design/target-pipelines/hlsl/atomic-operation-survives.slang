//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=53a2379796e273cdb95b30642542c77045c0b9200b2085bfda75a33dd67487c6
//META: purpose=An atomic operation on a UAV survives Phase C (validateAtomicOperations runs for non-SPIRV targets) and emits as the HLSL interlocked intrinsic.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Phase C table lists `validateAtomicOperations` for non-SPIRV
// targets. HLSL accepts the validated IR, and the HLSL emitter
// chooses the matching `InterlockedAdd(...)` intrinsic on the
// UAV access.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    InterlockedAdd(outBuf[0], int(tid.x));
}

// CHECK: InterlockedAdd(
