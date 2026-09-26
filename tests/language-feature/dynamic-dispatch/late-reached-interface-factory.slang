// Regression test for the dynamic-dispatch miscompile reported on
// https://github.com/shader-slang/slang/pull/12935 (issuecomment-5827779078).
//
// `getLight` is first reached from `computeMain`, so the type-flow pass rewrites its declared
// return type to a tagged union in an early fixpoint iteration. `lightIntersect` becomes reachable
// only in a later iteration, through the `IGeometry.intersectAndAccept` default method, and its
// call to `getLight` sees that already-specialized return type. The dispatch for
// `light.intersect(t)` must unpack the light's AnyValue payload into the concrete receiver.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -stage compute -profile sm_6_7 -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1

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

//TEST_INPUT: set lightTypes = ubuffer(data=[0 1 1 0], stride=4)
StructuredBuffer<uint> lightTypes;
//TEST_INPUT: set lightValues = ubuffer(data=[3.0 5.0 7.0 2.0], stride=4)
StructuredBuffer<float> lightValues;
//TEST_INPUT: set geometryTypes = ubuffer(data=[1 1 0 1], stride=4)
StructuredBuffer<uint> geometryTypes;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

ILight getLight(uint id)
{
    LightData data = { lightTypes[id], lightValues[id] };
    return createDynamicObject<ILight>(data.type, data);
}

bool lightIntersect(float3 origin, out float t)
{
    ILight light = getLight(uint(origin.x));
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
    static void intersectAndAccept<H : IHandler>(float3 origin, inout H handler, inout H.State state)
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
    static bool intersect(float3 origin, out float t) { return lightIntersect(origin, t); }
}

struct Handler : IHandler
{
    typealias State = float;
    [mutating]
    void accept(float t, inout State state) { state = t; }
}

bool occluded(uint id, float3 origin)
{
    float state = 0.0;
    Handler handler = {};
    IGeometry geometry = createDynamicObject<IGeometry>(geometryTypes[id], 0u);
    geometry.intersectAndAccept(origin, handler, state);
    return state > 0.0;
}

//TEST_INPUT: type_conformance Light:ILight = 0
//TEST_INPUT: type_conformance SurfaceLight:ILight = 1
//TEST_INPUT: type_conformance A:IGeometry = 0
//TEST_INPUT: type_conformance B:IGeometry = 1

// HLSL: Light_intersect_0(unpackAnyValue8_{{[0-9]+}}(
// HLSL: SurfaceLight_intersect_0(unpackAnyValue8_{{[0-9]+}}(
// SPV: OpEntryPoint
// DXIL: define void @computeMain()

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Thread 0: B hits a Light, which never intersects.
    // Thread 1: B hits a SurfaceLight (t = 5), so the output is its radiance, 5 * 2.
    // Thread 2: A intersects at t = length(origin) = 2; the output is SurfaceLight radiance 7 * 2.
    // Thread 3: B hits a Light, which never intersects.
    float light = getLight(tid.x).radiance();
    outputBuffer[tid.x] = occluded(tid.x, float3(tid)) ? light : 0.0;
    // CHECK: type: float
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 10.0
    // CHECK-NEXT: 14.0
    // CHECK-NEXT: 0.0
}
