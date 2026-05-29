//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test for issue #9835: Error when ALL implementations of an interface
// contain self-referential fields (no base case for AnyValue size calculation).
//
// The check is done at IR level during AnyValue size inference.

// CHECK: error[E41002]: interface has cyclic dependency on itself

interface IFoo
{
    float compute(float x);
}

// ALL implementations are self-referential - this is the error case.
// There's no base case to calculate the AnyValue size.

export struct BadImpl1 : IFoo
{
    IFoo inner;  // Self-referential
    float compute(float x) { return inner.compute(x) + 1.0f; }
}

export struct BadImpl2 : IFoo
{
    IFoo other;  // Also self-referential
    float compute(float x) { return other.compute(x) * 2.0f; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // TODO: Should dis-allow this type of createDynamicObject of a struct whose members include other
    // interface types, because the size of an interface type can vary depending on the implementations, 
    // and there's no meaningful mapping from a blob of data to such a type.
    //
    IFoo f = createDynamicObject<IFoo>(0, 1.0f);
    outputBuffer[0] = f.compute(1.0f);
}
