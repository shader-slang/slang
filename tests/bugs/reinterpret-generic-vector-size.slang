// Regression for #11289: `lowerReinterpret` can see a vector whose length is
// still a generic value parameter when generic methods are retained through
// witness-table/dynamic-dispatch paths. The compiler must treat that layout
// size as unresolved instead of dereferencing the non-literal count as an
// integer.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry computeMain -stage compute -conformance "Impl:IReinterpreter=1"
//TEST:SIMPLE: -target dxil -profile cs_6_0 -entry computeMain -stage compute -conformance "Impl:IReinterpreter=1"
//TEST:SIMPLE(filecheck=CHECK_CUDA): -target cuda -entry computeMain -stage compute -conformance "Impl:IReinterpreter=1"

RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IReinterpreter
{
    float run<let N : int>(vector<float, N> value);
}

struct Impl : IReinterpreter
{
    float scale;

    float run<let N : int>(vector<float, N> value)
    {
        return reinterpret<vector<float, N>>(value)[0] * scale;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl impl;
    impl.scale = 1.0;

    IReinterpreter obj = createDynamicObject<IReinterpreter, Impl>(1, impl);
    outputBuffer[0] = obj.run(float2(1.0, 2.0));
}

// CHECK_HLSL: RWStructuredBuffer<float > outputBuffer
// CHECK_CUDA: struct Impl
