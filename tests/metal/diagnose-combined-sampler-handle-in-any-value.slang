// A combined texture-sampler handle (`Sampler2D.Handle`) cannot be packed into an AnyValue
// on Metal. Metal lowers it to a `{ texture, sampler }` struct of two 64-bit resource IDs,
// but AnyValue marshalling would treat it as a single 8-byte handle and cast the struct to
// and from `ulong`, which is not valid MSL. Combined handles are not supported on Metal
// (#11540), so we report E41014 instead. Each type is diagnosed once, at its declaration
// when it has one.
//
// The TexLight lanes cover a closed set of conformers (`makeLight`), the `DescriptorHandle`
// spelling, and an open-world `createDynamicObject` with one or two conformers. The
// `reinterpret` and `throws` lanes cover the other two producers of AnyValue packing. A bare
// handle has no user declaration, so those report at the `reinterpret` call, and at the
// functions that pack and unpack the `throws` result.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute -DCLOSED_SET
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute -DCLOSED_SET -DUSE_DESCRIPTOR_HANDLE_SPELLING
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute -DDYNAMIC_OBJECT -conformance "TexLight:ILight=0" -conformance "ConstLight:ILight=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute -DDYNAMIC_OBJECT -DSINGLE_CONFORMER -conformance "TexLight:ILight=0"
//DIAGNOSTIC_TEST:SIMPLE(diag=REINTERPRET): -target metal -entry computeMain -stage compute -DREINTERPRET
//DIAGNOSTIC_TEST:SIMPLE(diag=THROWS): -target metal -entry computeMain -stage compute -DTHROWS

// A combined handle that never reaches AnyValue packing still compiles on Metal, and the
// closed-set, `reinterpret` and `throws` sources still compile for CUDA.
//TEST:SIMPLE(filecheck=STATIC): -target metal -entry computeMain -stage compute -DSTATIC_DISPATCH
//STATIC: computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -DCLOSED_SET
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -DREINTERPRET
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -DTHROWS
//CUDA: computeMain

RWStructuredBuffer<float4> outBuf;

#if defined(CLOSED_SET) || defined(DYNAMIC_OBJECT) || defined(STATIC_DISPATCH)

interface ILight
{
    float4 eval(float2 uv);
}

export struct TexLight : ILight
//CHECK:      ^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^ type 'TexLight' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
#ifdef USE_DESCRIPTOR_HANDLE_SPELLING
    DescriptorHandle<Sampler2D> tex;
#else
    Sampler2D.Handle tex;
#endif
    float4 eval(float2 uv) { return tex.SampleLevel(uv, 0); }
}

#ifndef SINGLE_CONFORMER
export struct ConstLight : ILight
{
    float4 c;
    float4 eval(float2 uv) { return c; }
}
#endif

#if defined(STATIC_DISPATCH)
StructuredBuffer<TexLight> texLights;

float4 evalLight<L : ILight>(L light)
{
    return light.eval(float2(0.5));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[tid.x] = evalLight(texLights[tid.x]);
}
#else

#ifdef CLOSED_SET
StructuredBuffer<TexLight> texLights;

ILight makeLight(uint i)
{
    if ((i & 1) == 1)
        return texLights[i];
    ConstLight c;
    c.c = float4(i);
    return c;
}
#else
uniform int dynId;

ILight makeLight(uint i)
{
    return createDynamicObject<ILight>(dynId, 0);
}
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ILight l = makeLight(tid.x);
    outBuf[tid.x] = l.eval(float2(0.5));
}
#endif

#elif defined(REINTERPRET)

StructuredBuffer<Sampler2D.Handle> handles;
RWStructuredBuffer<uint4> bitsOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bitsOut[tid.x] = reinterpret<uint4>(handles[tid.x]);
//REINTERPRET:                         ^ type cannot be packed for dynamic dispatch
//REINTERPRET:                         ^ type 'DescriptorHandle<Sampler2D>' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
}

#elif defined(THROWS)

enum LookupError
{
    Missing
}

StructuredBuffer<Sampler2D.Handle> handles;

Sampler2D.Handle lookup(uint i) throws LookupError
//THROWS:        ^^^^^^ type cannot be packed for dynamic dispatch
//THROWS:        ^^^^^^ type 'DescriptorHandle<Sampler2D>' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    if (i == 7)
        throw LookupError.Missing;
    return handles[i];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
/*THROWS:
     ^^^^^^^^^^^ type cannot be packed for dynamic dispatch
     ^^^^^^^^^^^ type 'DescriptorHandle<Sampler2D>' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
*/
{
    do
    {
        let h = try lookup(tid.x);
        outBuf[tid.x] = h.SampleLevel(float2(0.5), 0);
    }
    catch (e: LookupError)
    {
        outBuf[tid.x] = float4(0);
    }
}

#endif
