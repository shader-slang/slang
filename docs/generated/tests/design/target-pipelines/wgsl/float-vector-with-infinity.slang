//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
//META: purpose=+inf and -inf in a float vector emit via the _slang_getInfinity helper because WGSL has no inf literal
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL has no surface syntax for IEEE-754 infinities; the emitter
// synthesises a helper function _slang_getInfinity that returns
// the value via 1.0f / 0.0f.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float2> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float pos = 1.0f / 0.0f;
    float neg = -1.0f / 0.0f;
    outBuf[tid.x] = float2(pos, neg);
}

// CHECK: fn _slang_getInfinity
// CHECK: _slang_getInfinity(true)
// CHECK: _slang_getInfinity(false)
