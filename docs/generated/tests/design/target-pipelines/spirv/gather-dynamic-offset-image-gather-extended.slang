//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#capabilities-and-extensions-decided-at-emit-time
//META: doc_section_digest=a1bda696e594d0d008416384d3cd2b7b449a57153c72f78ceef01aa513a35c5e
//META: purpose=A Gather with a runtime-computed offset falls through to the Offset image operand and requests OpCapability ImageGatherExtended.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the dynamic half of the same emit-time decision: only a
// non-constant offset needs the extended-gather capability. The offset is
// derived from the dispatch thread id so the emitter cannot recognize it as a
// constant; CHECK asserts both that OpCapability ImageGatherExtended is
// declared and that the gather uses the plain Offset operand rather than
// ConstOffset.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int2 off = int2(tid.xy);
    outBuf[tid.x] = tex.GatherRed(samp, float2(0.5, 0.5), off);
}

// CHECK-DAG: OpCapability ImageGatherExtended
// CHECK-DAG: OpImageGather {{.*}} Offset %{{[a-zA-Z_0-9]+}}
