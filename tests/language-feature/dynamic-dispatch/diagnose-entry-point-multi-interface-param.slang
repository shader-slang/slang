// Multiple interface-typed entry point parameters without conformances should
// each produce a separate E50100 diagnostic. This verifies that the per-param
// dedup set in diagnoseEntryPointInterfaceParamIfNeeded emits one error per
// distinct interface parameter, not one per ExtractExistential* instruction.

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
