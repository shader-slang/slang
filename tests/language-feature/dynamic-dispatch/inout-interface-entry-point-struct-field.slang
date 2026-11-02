// An interface-typed field of an entry point's `uniform` struct parameter
// keeps the existential's buffer layout (RTTI, witness table, any-value).
// A callee's `inout` / `out` parameter is specialized to a compact tagged
// union, so passing the field by reference must go through a temporary of
// the callee's type. Before #14152 was fixed, the field's address was passed
// directly, which DXC and the Metal compiler rejected. The same holds for the
// field of a struct in a structured buffer.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST(dxc):SIMPLE(filecheck=DXIL): -target dxil -profile cs_6_0 -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1"

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
    IFoo bar;
    float scale;
}

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<Params> gParams;

void takeRef(inout IFoo f)
{
    outputBuffer[1] = f.getValue();
}

void copyOut(out IFoo dst, IFoo src)
{
    dst = src;
}

// The calls take temporaries, not the fields' addresses.
// HLSL: void computeMain
// HLSL: takeRef_{{[0-9]+}}(_S{{[0-9]+}})
// HLSL: copyOut_{{[0-9]+}}(_S{{[0-9]+}},
// HLSL: takeRef_{{[0-9]+}}(_S{{[0-9]+}})

// DXIL: define void @computeMain

// METAL: void computeMain
// METAL: takeRef_{{[0-9]+}}(&_S{{[0-9]+}},
// METAL: copyOut_{{[0-9]+}}(&_S{{[0-9]+}},
// METAL: takeRef_{{[0-9]+}}(&_S{{[0-9]+}},

// METALLIB: @computeMain

[numthreads(1, 1, 1)]
void computeMain(uniform Params p)
{
    outputBuffer[0] = p.foo.getValue() * p.scale;
    takeRef(p.foo);
    copyOut(p.bar, p.foo);
    outputBuffer[2] = p.bar.getValue();
    takeRef(gParams[0].foo);
}
