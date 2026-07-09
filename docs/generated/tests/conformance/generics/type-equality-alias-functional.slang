//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A type equality constraint `where T == int` causes T to be treated as int for all purposes inside the generic body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Type-Checking rule that an equality constraint `T == U` makes T
// an alias for U: with `where T == int`, an int literal may be added to a value
// of type T and the result assigned to an int. The function is called at T=int;
// CHECK asserts 15, proving the body type-checked int operations on T solely
// because of the equality constraint.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int addTen<T>(T x) where T == int
{
    int r = x + 10;
    return r;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = addTen(5);
}
// CHECK: 15
