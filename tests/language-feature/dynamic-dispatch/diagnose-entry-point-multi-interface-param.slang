// Multiple interface-typed entry point parameters without conformances should
// each produce a separate E50100 diagnostic. This verifies that the per-param
// dedup set in diagnoseEntryPointInterfaceParamIfNeeded emits one error per
// distinct interface parameter, not one per ExtractExistential* instruction.
//
// CUDA is the primary target: it skips collectEntryPointUniformParams and
// exercises the new diagnoseEntryPointInterfaceParamIfNeeded helper, which
// emits exactly one diagnostic per param with source locations and interface
// names. SPIRV/HLSL go through analyzeLoad instead and just verify E50100.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain

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
    // CHECK: error[E50100]
    // CUDA-DAG: interface 'IFoo'
    // CUDA-DAG: interface 'IBar'
    outputBuffer[0] = foo.getValue() + bar.weight();
}
