// Default-initializing a link-time struct alias (`export struct Foo : IFoo = FooImpl;`) that
// resolves to a non-empty struct must produce that struct's field-wise default value, not an
// ill-formed zero-operand IRMakeStruct. Like an associated type, such an alias has no concrete
// fields of its own during lowering and only resolves during linking, so it must lower to a
// deferred DefaultConstruct. Regression for shader-slang/slang#12708 (the link-time-struct
// variant of the associated-type crash).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IFoo
{
    float get();
}

struct FooImpl : IFoo
{
    float v;
    float get() { return v; }
}

export struct Foo : IFoo = FooImpl;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo value = {};
    outputBuffer[0] = value.get();
    // CHECK: 0.0
}
