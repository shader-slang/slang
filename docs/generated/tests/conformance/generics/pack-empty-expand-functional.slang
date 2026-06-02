//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A type parameter pack can be bound to an empty sequence (zero arguments); when no `nonempty` constraint is declared, an empty pack is a valid specialization.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: when a generic construct is specialized, a (possibly empty) sequence
// of type arguments is bound to the parameter pack. This tests the empty case (0 args).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addHelper(inout int acc, int v) { acc += v; }

int sumInts<each T>(expand each T terms) where T == int
{
    int acc = 0;
    expand addHelper(acc, each terms);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    // Empty pack: zero arguments
    outputBuffer[0] = sumInts();
}
// CHECK: 0
