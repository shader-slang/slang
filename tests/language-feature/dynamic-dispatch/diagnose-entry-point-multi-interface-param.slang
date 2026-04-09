// Multiple interface-typed entry point parameters without conformances should
// produce E50100 diagnostics. For most targets, the error surfaces through
// collectEntryPointUniformParams and the normal typeflow path. For CUDA compute,
// which skips that pass, the typeflow pass's diagnoseEntryPointInterfaceParamIfNeeded
// catches the missing conformances directly.

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
