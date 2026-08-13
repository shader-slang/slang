//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=A NaN-producing expression (0.0/0.0) emits via the _slang_getNan helper because WGSL has no nan literal
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL has no surface syntax for IEEE-754 NaN; the emitter
// synthesises a helper function _slang_getNan that returns the
// value via 0.0f / 0.0f.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float nan = 0.0f / 0.0f;
    outBuf[tid.x] = nan;
}

// CHECK: fn _slang_getNan
// CHECK: _slang_getNan()
