//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=A type parameter pack bound to zero arguments expands to no calls (empty pack, length 0).
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the empty-pack edge: `sumInts<each T>` is called with no
// arguments, so the pack has length 0 and `expand sumHelper(...)` produces no
// calls at all. The accumulator is pre-seeded with 7; CHECK asserts 7,
// confirming the expansion over an empty pack ran zero iterations.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void sumHelper(inout int acc, int term)
{
    acc += term;
}

int sumInts<each T>(expand each T terms) where T == int
{
    int acc = 7;
    expand sumHelper(acc, each terms);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumInts();
}
// CHECK: 7
