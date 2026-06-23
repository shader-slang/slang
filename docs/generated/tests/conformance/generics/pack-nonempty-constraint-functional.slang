//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=`where nonempty(T)` constraint is accepted and a function using it compiles and executes correctly when the pack is non-empty; the pack-expansion processes all elements.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `where nonempty(T)` is accepted by the compiler and that pack expansion
// works correctly when the pack is non-empty. Calls `sumIntsNonempty(10, 20, 30)`;
// CHECK asserts 60, confirming all three elements were processed.
// NOTE: `__first` crashes the compiler (see finding: pack-query-first-last-compiler-crash).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addHelper(inout int acc, int v) { acc += v; }

int sumIntsNonempty<each T>(expand each T args)
    where nonempty(T)
    where T == int
{
    int acc = 0;
    expand addHelper(acc, each args);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[0] = sumIntsNonempty(10, 20, 30);
}
// CHECK: 60
