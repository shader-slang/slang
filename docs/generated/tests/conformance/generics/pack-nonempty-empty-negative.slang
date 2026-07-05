//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=A `where nonempty(T)` constraint rejects specialization when the pack is bound to zero arguments.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-emptiness guard's negative side: `sumNonempty<each T> where
// nonempty(T)` requires the pack to be non-empty at specialization time.
// Calling it with no arguments binds the pack to length 0, violating the
// constraint. CHECK asserts E39999 "could not specialize generic" fires on the
// empty call (non-exhaustive skips the trailing "see declaration" note).

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
    //CHECK: E39999
}
