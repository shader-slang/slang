// A struct containing an interface-typed field used as an entry point
// parameter should generate dynamic dispatch code when conformances
// are registered.
//
// SPIRV and WGSL targets hit a separate assertion in storage-to-logical
// type legalization and are left disabled until that is resolved.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//DISABLED_TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

[anyValueSize(16)]
interface IFoo
{
    float getValue();
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

struct Params
{
    IFoo foo;
    float scale;
}

RWStructuredBuffer<float> outputBuffer;

void takeRef(inout IFoo f, RWStructuredBuffer<float> buf)
{
    buf[1] = f.getValue();
}

// HLSL: s_dispatch_IFoo{{.*}}getValue
// HLSL: void computeMain
// GLSL: void main()
// MTL: void computeMain
[numthreads(1, 1, 1)]
void computeMain(uniform Params p)
{
    outputBuffer[0] = p.foo.getValue() * p.scale;
    takeRef(p.foo, outputBuffer);
}
