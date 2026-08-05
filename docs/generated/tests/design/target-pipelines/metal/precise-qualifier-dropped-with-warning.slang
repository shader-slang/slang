//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Negative: a `precise` local is dropped on Metal with warning E56005 instead of emitting the HLSL/GLSL keyword the Apple compiler would reject.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal override of the shared temp-modifier emission. MSL
// has no `precise` keyword, so emitting it verbatim would make the
// generated source uncompilable; instead the emitter reports that the
// qualifier is being ignored. The test pins the stable code E56005 plus
// the caret-anchored span the runner reports, and E40100 for the
// entry-point rename, which are all the diagnostics this compile emits.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    precise float p = float(tid.x) * 2.0f + 1.0f;
    outBuf[tid.x] = p;
}

//CHECK: E56005
//CHECK: E40100
