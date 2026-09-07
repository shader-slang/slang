//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=A `where countof(T) == N` constraint does not infer the value argument N from the number of pack arguments supplied.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-inference rule for pack-count constraints: the call supplies
// three pack arguments but leaves N unbound, and the constraint is not used
// backwards to deduce N = 3. CHECK asserts the call is rejected because the
// generic could not be specialized for the given arguments, proving N must be
// bound explicitly or inferred by ordinary generic-argument rules.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumExactly<let N : int, each T>(expand each T args)
    where countof(T) == N
    where T == int
{
    int acc = 0;
    expand addHelper(acc, each args);
    return acc;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumExactly(1, 2, 3);
    //CHECK: E39999
}
