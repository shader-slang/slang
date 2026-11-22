//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
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
