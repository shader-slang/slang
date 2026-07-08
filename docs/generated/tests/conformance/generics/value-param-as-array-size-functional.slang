//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A generic value parameter is usable only as its declared compile-time constant (e.g. an array bound / loop bound); no other assumption is made about it.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Type-Checking claim that no assumptions are made about a generic
// value parameter beyond its declared type: `count<let N : uint>()` uses N only
// as an ordinary uint compile-time constant to size a loop and returns it.
// Specialized as `count<5>`; CHECK asserts 5, proving the value parameter
// behaves as its plain declared-type constant.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

uint count<let N : uint>()
{
    uint acc = 0;
    for (uint i = 0; i < N; ++i)
        acc += 1;
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = count<5>();
}
// CHECK: 5
