// Two global interface parameters of different interface types exercise the
// ordering and interaction in `seedGlobalScope` / `specializeGlobalInsts`
// (both iterate over all module-scope instructions).  With a single global the
// ordering is trivial; with two independent interfaces any cross-contamination
// would surface here.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "ImplX:IBar=0"
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "ImplX:IBar=0"

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

[anyValueSize(16)]
interface IBar
{
    int getCount();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

struct ImplX : IBar
{
    int n;
    int getCount() { return n + 1; }
}

ConstantBuffer<IFoo> gFoo;
ConstantBuffer<IBar> gBar;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = gFoo.getValue() + float(gBar.getCount());
}

// Each interface must generate its own dispatch function.  `IFoo` has two
// impls (dispatch switch), `IBar` has one (monomorphized).
// HLSL-DAG: ImplA_getValue
// HLSL-DAG: ImplB_getValue
// HLSL-DAG: ImplX_getCount
// HLSL-DAG: s_dispatch_IFoo_getValue
// HLSL-DAG: switch(
// HLSL: void computeMain

// SPIRV: OpEntryPoint
// SPIRV: OpSwitch
