//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#pack-queries
//META: doc_section_digest=c04a621bcf00069773d305dfdf4582e5e2e0ce23042884e2675ec84c546547fe
//META: purpose=`__trimFirst(P)` applied to an empty pack is total and yields an empty pack rather than an error.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the totality claim for `__trimFirst`: the pack is bound to
// zero arguments, so `__trimFirst(args)` is applied to an empty pack. Totality
// means this is well-formed and yields another empty pack, so the surrounding
// expansion produces no calls at all. The accumulator is pre-seeded with 7;
// CHECK asserts 7, proving the query neither errored nor produced elements.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addHelper(inout int acc, int v)
{
    acc += v;
}

int sumTrimFirst<each T>(expand each T args) where T == int
{
    int acc = 7;
    expand addHelper(acc, each __trimFirst(args));
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumTrimFirst();
}
// CHECK: 7
