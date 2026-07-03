//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#examples
//META: doc_section_digest=2843e3e730992c3b4a58e5dcb338c66a5db5be57ab33d9c88e766fe31406de61
//META: purpose=A generic function taking an `IFunc<T,T,T>` parameter can be called with a concrete struct implementing IFunc.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Generic function with type parameter" example: a struct
// `Adder` conforms to `IFunc<int,int,int>` and is passed to
// `performOp<T>(IFunc<T,T,T> op, T a, T b)`. The kernel evaluates
// performOp(adder, 234, 456); CHECK asserts 234+456+bias(5)=695, proving the
// interface-typed parameter dispatched to Adder's operator().

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
void computeMain()
{
    Adder addWithBias = { 5 };
    outputBuffer[0] = performOp(addWithBias, 234, 456);
}
// CHECK: 695
