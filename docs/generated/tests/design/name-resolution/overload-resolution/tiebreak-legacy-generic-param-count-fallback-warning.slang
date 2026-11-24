//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#tie-breaking-comparator
//META: doc_section_digest=764e196b19a5dd0e0f2c516c498be250f0fdaba7e58c28f2ac820f88be1e5466
//META: purpose=Before Slang 202c a tie that survives every comparator step is resolved by the single candidate with the fewest required generic parameters, with a deprecation warning.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the legacy generic-parameter-count fallback. The two `select`
// overloads tie on every ordinary criterion for a float3 argument; at the
// default language version the one with fewer required generic parameters is
// chosen and the compiler warns. CHECK pins the warning code, and the compile
// still succeeds (default exit code 0).

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute

int select<T>(vector<T, 3> value) { return 1; }
int select<T, let N : int>(vector<T, N> value) { return 2; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    // CHECK: E40021
    outputBuffer[0] = select(float3(1, 2, 3));
}
