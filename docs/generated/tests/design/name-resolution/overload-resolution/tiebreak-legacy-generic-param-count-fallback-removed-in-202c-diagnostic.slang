//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=764e196b19a5dd0e0f2c516c498be250f0fdaba7e58c28f2ac820f88be1e5466
//META: purpose=From Slang 202c the legacy generic-parameter-count fallback no longer applies, so the same tied pair of generic overloads is reported as an ambiguous call.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fallback is version-gated. Same overload pair as the legacy
// test, compiled with -std 202c: the tied frontier is not resolved and the
// call fails with the ambiguous-call error (E39999), exit code -1.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -std 202c -target hlsl -entry main -stage compute

int select<T>(vector<T, 3> value) { return 1; }
int select<T, let N : int>(vector<T, N> value) { return 2; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    // CHECK: E39999
    // CHECK: ambiguous call to 'select'
    outputBuffer[0] = select(float3(1, 2, 3));
}
