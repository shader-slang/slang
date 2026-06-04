//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic function can take an IFunc-typed parameter; the concrete implementing struct is passed and called through the interface.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Generic function with type parameter).
// `performOp` takes an `IFunc<T, T, T>` parameter and a `Adder` struct implements it.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

func performOp<T>(IFunc<T, T, T> binaryOp, T a, T b) -> T
{
    return binaryOp(a, b);
}

func add2<T : IArithmetic>(T a, T b) -> T
{
    return a + b;
}

struct Adder : IFunc<int, int, int>
{
    int bias;
    int operator()(int a, int b)
    {
        return add2(add2(a, b), bias);
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    Adder addTwoWithBias;
    addTwoWithBias.bias = 5;
    outputBuffer[0] = performOp(addTwoWithBias, 234, 456);
}
// CHECK: 695
