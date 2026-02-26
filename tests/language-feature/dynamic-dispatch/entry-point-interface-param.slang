// Entry point parameters can be interface-typed when type conformances are
// registered. The compiler generates dynamic dispatch code for method calls
// on these parameters, packing them as existential tuples in a constant buffer.
// Multiple interface-typed parameters are supported at the same entry point.
//
// Most targets are disabled due to an assertion failure in IRUse::set() on
// x86_64 (#10266). Re-enable when that bug is fixed.

//DISABLED_TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//DISABLED_TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//DISABLED_TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//DISABLED_TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//DISABLED_TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1"
//DISABLED_TEST:SIMPLE(filecheck=REPORT): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "BarImplA:IBar=0" -conformance "BarImplB:IBar=1" -report-dynamic-dispatch-sites

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

struct BarImplA : IBar
{
    float w;
    float weight() { return w; }
}

struct BarImplB : IBar
{
    float w;
    float weight() { return w * 0.5; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform IFoo foo, uniform IBar bar)
{
    // REPORT-DAG: ([[# @LINE+1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[0] = foo.getValue();
    // REPORT-DAG: ([[# @LINE+1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    outputBuffer[1] = bar.weight();
}

// SPIRV: OpEntryPoint
// HLSL: void computeMain
// GLSL: void main()
// CUDA: void computeMain
// MTL: void computeMain
// WGSL: fn computeMain
