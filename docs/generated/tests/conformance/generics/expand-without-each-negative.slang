//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#type-parameter-packs-type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=An expand-expression containing no each-expression is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the converse enclosure rule: an expand-expression must contain at
// least one each-expression, otherwise there is no pack telling the expansion
// how many elements to produce. The `expand` below wraps a call whose arguments
// are all ordinary values. CHECK asserts E30105 fires on it, proving the empty
// expansion is diagnosed rather than expanded zero or one times.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumInts<each T>(expand each T args) where T == int
{
    int acc = 0;
    expand addHelper(acc, 5);
    //CHECK: E30105
    return acc;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumInts(1, 2);
}
