//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=2a4c97aeaacd2a9728a83c1944ef99c7efe30fe3fe60ef075d8d47044f03eefc
//META: purpose=`lowerBitCast` renders `asuint(float)` as MSL's `as_type<uint>(...)` reinterpret builtin.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal rendering chosen for the IR bitcast that `asuint`
// produces: MSL spells bit reinterpretation as `as_type<T>`, not a C-style
// cast or a union punning helper. The source bits come from a `uniform`
// so the folder cannot pre-compute the reinterpretation, and the result is
// stored to a buffer so it escapes DCE; CHECK asserts the `as_type<uint>(`
// call appears.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform float f;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asuint(f) + tid.x;
}

// CHECK: as_type<uint>(
