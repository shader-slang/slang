//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Boundary: int MIN (-2147483648) is emitted as the decimal literal wrapped in int(...) cast on HLSL.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: int MIN. The emitter writes the literal as
// `int(-2147483648)` so the type narrows cleanly under DXC.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = -2147483648;
}

// CHECK: -2147483648
