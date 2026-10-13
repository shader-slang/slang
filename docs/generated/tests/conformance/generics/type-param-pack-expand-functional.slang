//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#type-parameter-pack
//META: doc_section_digest=f0666c2d444ef1f203d4941f9f3316ff8c39bef6547ab07edd2666e6a16554d6
//META: purpose=A type parameter pack expanded with `expand ... each ...` produces one call per pack element, in order.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Type parameter pack" example: `sumInts<each T>(expand
// each T terms)` expands `expand sumHelper(acc, each terms)` into one
// sumHelper call per element of the pack. Called with seven int arguments;
// CHECK asserts the accumulated sum 1..7 == 28, proving the expand/each
// construct generated a call for every pack member.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void sumHelper(inout int acc, int term)
{
    acc += term;
}

int sumInts<each T>(expand each T terms) where T == int
{
    int acc = 0;
    expand sumHelper(acc, each terms);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumInts(1, 2, 3, 4, 5, 6, 7);
}
// CHECK: 28
