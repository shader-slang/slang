//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=a1bda696e594d0d008416384d3cd2b7b449a57153c72f78ceef01aa513a35c5e
//META: purpose=A Gather with a compile-time-constant offset uses the ConstOffset image operand and does not request the ImageGatherExtended capability.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constant half of the emit-time ImageGatherExtended decision.
// A literal int2 offset can travel in the ConstOffset image operand, which
// core SPIR-V allows, so no extra capability may be requested. CHECK asserts
// the gather uses ConstOffset and CHECK-NOT asserts the capability is absent
// from the whole module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.GatherRed(samp, float2(0.5, 0.5), int2(1, 1));
}

// CHECK-NOT: OpCapability ImageGatherExtended
// CHECK: OpImageGather {{.*}}ConstOffset
