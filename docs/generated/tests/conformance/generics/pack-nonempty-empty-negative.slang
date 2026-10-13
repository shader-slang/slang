//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-queries
//META: doc_section_digest=c04a621bcf00069773d305dfdf4582e5e2e0ce23042884e2675ec84c546547fe
//META: purpose=A `where nonempty(T)` constraint rejects specialization when the pack is bound to zero arguments.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-emptiness guard's negative side: `sumNonempty<each T> where
// nonempty(T)` requires the pack to be non-empty at specialization time.
// Calling it with no arguments binds the pack to length 0, violating the
// constraint. CHECK asserts E30440 "generic constraint not satisfied" fires on
// the empty call, naming the 'nonempty(T)' constraint (non-exhaustive skips
// the trailing constraint-declaration and function-declaration notes).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumNonempty<each T>(expand each T args) where nonempty(T) where T == int
{
    int acc = 0;
    expand addHelper(acc, each args);
    return acc;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumNonempty();
//CHECK:                         ^ could not satisfy the generic constraint 'nonempty(T)'
    //CHECK: E30440
}
