//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#pack-queries
//META: doc_section_digest=c04a621bcf00069773d305dfdf4582e5e2e0ce23042884e2675ec84c546547fe
//META: purpose=`__trimFirst(P)` is a total pack-query operation that drops the first element and yields the remaining pack.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pack-query claim that `__trimFirst(P)` is total: it removes the
// first pack element and returns the rest. `sumTrimFirst<each T>` expands
// `addHelper(acc, each __trimFirst(args))`, so calling with (10, 20, 30) drops
// 10 and sums 20 + 30. CHECK asserts 50, proving __trimFirst discarded exactly
// the leading element.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumTrimFirst<each T>(expand each T args) where T == int
{
    int acc = 0;
    expand addHelper(acc, each __trimFirst(args));
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumTrimFirst(10, 20, 30);
}
// CHECK: 50
