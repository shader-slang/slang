// Test that using ByteAddressBuffer.Load<T>/Store<T> with an interface type
// emits a diagnostic instead of crashing the compiler.
// GitHub issue: #10376

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -conformance "ImplA:IFoo=1"

[anyValueSize(8)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

RWByteAddressBuffer byteBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ImplA a;
    a.v = 7.0;
    IFoo fooA = a;
    byteBuffer.Store<IFoo>(0, fooA);

    IFoo loaded = byteBuffer.Load<IFoo>(0);
    outputBuffer[0] = loaded.getValue();
}

// CHECK: error[E33180]
