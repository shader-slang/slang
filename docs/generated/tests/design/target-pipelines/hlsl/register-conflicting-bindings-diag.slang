//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
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
