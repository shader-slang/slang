//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-count-constraints
//META: doc_section_digest=4cab677003373b1406871adb52be6c61cf027b707a49fe5f17a102d4ea0eaf9f
//META: purpose=A `where countof(T) == N` constraint accepts a specialization whose pack argument list has exactly N elements.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pack-count constraint's accepting side: `sumExactly<let N : int,
// each T>` declares `where countof(T) == N`, and the call binds N explicitly to
// 3 while supplying a three-element pack. The body seeds the accumulator with
// N * 100 so the value proves N was bound to 3, then expands over the pack.
// CHECK asserts 306 (= 300 + 1 + 2 + 3), which is only reachable if the
// constraint admitted the specialization and the pack expanded over all three
// elements.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumExactly<let N : int, each T>(expand each T args)
    where countof(T) == N
    where T == int
{
    int acc = N * 100;
    expand addHelper(acc, each args);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumExactly<3>(1, 2, 3);
}
// CHECK: 306
