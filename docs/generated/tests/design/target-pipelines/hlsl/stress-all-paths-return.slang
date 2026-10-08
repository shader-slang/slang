//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Stress: every control-flow path returns explicitly; the emitter still produces a single void main entry point.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: all-paths-diverge. The kernel writes the buffer
// and returns on every branch. The emitter must still produce a
// valid `void main(...)` function and the standard numthreads
// attribute.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0) { outBuf[0] = 1; return; }
    if (tid.x == 1) { outBuf[1] = 2; return; }
    if (tid.x == 2) { outBuf[2] = 3; return; }
    outBuf[tid.x] = -1;
    return;
}

// CHECK: void main(
// CHECK: return
