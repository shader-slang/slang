// Regression test for https://github.com/shader-slang/slang/issues/12934 — reporter's standalone
// dynamic-light-factory reducer (no autodiff, no ray tracing).
//
// A generic geometry callback (`IGeometry.intersect_and_accept<H>`) and a second consumer in `main`
// share the dynamic light factory `LightSystem.get_light`. An earlier specialization phase lowers
// that factory's `ILight` result to an `IRTaggedUnionType`; the type-flow specialization pass then
// re-wrapped the already-lowered union into `UntaggedUnion(TypeSet(TaggedUnion(...)))`, which
// `analyzeExtractExistentialWitnessTable` rejected with "Unhandled info type" on every target.
//
// The already-lowered union must reach both consumers unchanged, so that both dispatch through
// `(tag, AnyValue)` to the concrete `Light` / `SurfaceLight` methods. The DXIL and PTX directives
// check more than the absence of an internal error: if the union info is dropped instead, the
// emitted code passes an `AnyValue` where a concrete light struct is expected, which DXC and NVRTC
// reject. SPIR-V output is validated when `SLANG_RUN_SPIRV_VALIDATION=1` is set.
//
// Load-bearing shape (do not "simplify"): the callback path (USE_CALLBACK), the factory's second
// consumer (USE_OUTER_LIGHT), and a non-inlined factory (INLINE_LIGHT_LOOKUP=0) are each required;
// `-DINLINE_LIGHT_LOOKUP=1`, `-DUSE_CALLBACK=0` and `-DUSE_OUTER_LIGHT=0` each compile even before
// the fix, so they are negative controls rather than tested directives.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -profile sm_6_7 -entry main -stage compute -conformance "A:IGeometry=0" -conformance "B:IGeometry=1" -conformance "Light:ILight=0" -conformance "SurfaceLight:ILight=1"
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -profile sm_6_7 -entry main -stage compute -conformance "A:IGeometry=0" -conformance "B:IGeometry=1" -conformance "Light:ILight=0" -conformance "SurfaceLight:ILight=1"
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry main -stage compute -conformance "A:IGeometry=0" -conformance "B:IGeometry=1" -conformance "Light:ILight=0" -conformance "SurfaceLight:ILight=1"
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry main -stage compute -conformance "A:IGeometry=0" -conformance "B:IGeometry=1" -conformance "Light:ILight=0" -conformance "SurfaceLight:ILight=1"

#ifndef INLINE_LIGHT_LOOKUP
#define INLINE_LIGHT_LOOKUP 0
#endif
#ifndef USE_CALLBACK
#define USE_CALLBACK 1
#endif
#ifndef USE_OUTER_LIGHT
#define USE_OUTER_LIGHT 1
#endif

interface ILight
{
    bool intersect(out float t);
    float radiance();
}

struct LightData
{
    uint type;
    float value;
}

struct Light : ILight
{
    LightData data;
    bool intersect(out float t)
    {
        t = 0.0;
        return false;
    }
    float radiance() { return data.value; }
}

struct SurfaceLight : ILight
{
    LightData data;
    bool intersect(out float t)
    {
        t = data.value;
        return t > 0.0;
    }
    float radiance() { return data.value * 2.0; }
}

struct LightSystem
{
    StructuredBuffer<LightData> lights;
#if INLINE_LIGHT_LOOKUP
    [__unsafeForceInlineEarly]
#endif
    ILight get_light(uint id)
    {
        LightData data = lights[id];
        return createDynamicObject<ILight>(data.type, data);
    }
}

LightSystem light_system;

bool light_intersect(float3 origin, out float t)
{
    ILight light = light_system.get_light(uint(origin.x));
    return light.intersect(t);
}

interface IHandler
{
    associatedtype State;
    [mutating]
    void accept(float t, inout State state);
}

interface IGeometry
{
    static bool intersect(float3 origin, out float t);
    static void intersect_and_accept<H : IHandler>(float3 origin, inout H handler, inout H.State state)
    {
        float t;
        if (This.intersect(origin, t))
            handler.accept(t, state);
    }
}

struct A : IGeometry
{
    static bool intersect(float3 origin, out float t)
    {
        t = length(origin);
        return t > 0.0;
    }
}

struct B : IGeometry
{
    static bool intersect(float3 origin, out float t) { return light_intersect(origin, t); }
}

struct Handler : IHandler
{
    typealias State = float;
    [mutating]
    void accept(float t, inout State state) { state = t; }
}

uniform uint geometry_type;
RWStructuredBuffer<float> result;

IGeometry geometry()
{
    return createDynamicObject<IGeometry>(geometry_type, 0u);
}

bool occluded(float3 origin)
{
    float state = 0.0;
    Handler handler = {};
#if USE_CALLBACK
    geometry().intersect_and_accept(origin, handler, state);
#else
    float t;
    if (geometry().intersect(origin, t))
        handler.accept(t, state);
#endif
    return state > 0.0;
}

// SPIRV: OpEntryPoint
// DXIL: define void @main()
// CUDA: __global__ void main
// PTX: .visible .entry main
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
#if USE_OUTER_LIGHT
    float light = light_system.get_light(tid.x).radiance();
#else
    float light = 1.0;
#endif
    result[tid.x] = occluded(float3(tid)) ? light : 0.0;
}
