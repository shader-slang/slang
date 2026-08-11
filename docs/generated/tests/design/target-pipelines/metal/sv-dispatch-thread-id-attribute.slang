//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=1b40e6975a461c5c97ead4202c2a70e579c2e6e882749ceb49d57be34bac586c
//META: purpose=`SV_DispatchThreadID` lowers to a `uint3` kernel parameter carrying Metal's `[[thread_position_in_grid]]` system-value attribute.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the varying-parameter half of `legalizeIRForMetal`: an HLSL
// system-value semantic has no meaning in MSL, so the parameter must be
// re-annotated with the equivalent Metal attribute. CHECK matches the
// parameter declaration and then, on the same emitted line, the
// `thread_position_in_grid` marker -- matched as a bare substring because
// FileCheck would read the literal `[[...]]` form as a variable reference.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: uint3 tid_{{[0-9]+}}
// CHECK-SAME: thread_position_in_grid
