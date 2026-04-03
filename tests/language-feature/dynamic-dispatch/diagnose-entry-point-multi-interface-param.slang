// Multiple interface-typed entry point parameters without conformances should
// produce E50100 diagnostics. All targets now go through
// collectEntryPointUniformParams, so the error surfaces through the normal
// typeflow specialization path.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

[anyValueSize(16)]
interface IBar
{
    float weight();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform IFoo foo, uniform IBar bar)
{
    // CHECK-COUNT-2: error[E50100]
    outputBuffer[0] = foo.getValue() + bar.weight();
}
