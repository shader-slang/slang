//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=06568c8014909b1a1fe61ae1dd314428312f177b6412e33a4e643d370a1fc6d9
//META: purpose=A generic type parameter with a default type (`T = int`) can be specialized without an explicit type argument.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the description claim that a type parameter may carry a default type
// via `= simple-type-spec`. `struct Box<T = int>` is used as `Box<>` with no
// argument, so T falls back to the default int. CHECK asserts the stored int
// (42) is readable, proving the default type filled the omitted argument.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Box<T = int>
{
    T v;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Box<> b;
    b.v = 42;
    outputBuffer[0] = b.v;
}
// CHECK: 42
