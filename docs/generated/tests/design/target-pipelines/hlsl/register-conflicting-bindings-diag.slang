//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Negative: two RWStructuredBuffers explicitly bound to the same register(u0) trigger an explicit-binding-overlap diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: conflicting bindings on the same register slot.
// The compiler must flag the overlap, since two resources cannot
// share a single u register.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> bufA : register(u0);
RWStructuredBuffer<float> bufB : register(u0);
//CHECK:                  ^^^^ explicit binding overlap
//CHECK:                  ^^^^ explicit binding for parameter 'bufB' overlaps with parameter 'bufA'
// CHECK: W39001

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[tid.x] = int(tid.x);
    bufB[tid.x] = float(tid.x);
}
