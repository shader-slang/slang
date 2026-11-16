// Regression test for a single concrete `ILight` conformance returned through an interface-typed
// function that is reached in two `specializeDynamicInsts` runs.
//
// `computeMain` calls `getLight` directly, so an earlier run rewrites its declared result type to a
// `TaggedUnionType`. `lightIntersect` is reached only in a later run, through the
// `IGeometry.intersectAndAccept` default method, and its call to `getLight` sees that rewritten
// type. If `makeInfoForConcreteType` wrapped it as a payload, compilation would stop with an internal
// error. The later `light.intersect(t)` must resolve to a direct `Light.intersect` call.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -conformance A:IGeometry=0 -conformance B:IGeometry=1
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -conformance A:IGeometry=0 -conformance B:IGeometry=1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -stage compute -conformance A:IGeometry=0 -conformance B:IGeometry=1

interface ILight
{
    bool intersect(out float t);
    float radiance();
}

struct Light : ILight
{
    float value;
    bool intersect(out float t)
    {
        t = value;
        return t > 0.0;
    }
    float radiance() { return value * 2.0; }
}

//TEST_INPUT: set lightValues = ubuffer(data=[0.0 5.0 7.0 0.0], stride=4)
StructuredBuffer<float> lightValues;
//TEST_INPUT: set geometryTypes = ubuffer(data=[1 1 0 1], stride=4)
StructuredBuffer<uint> geometryTypes;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

ILight getLight(uint id)
{
    Light light = { lightValues[id] };
    return light;
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

//TEST_INPUT: type_conformance A:IGeometry = 0
//TEST_INPUT: type_conformance B:IGeometry = 1

// The HLSL check requires a direct call on the value `getLight` returns rather than witness-based
// dispatch. The SPIR-V and DXIL runs only match the entry point, so they catch ill-typed output
// only when SPIR-V validation (`SLANG_RUN_SPIRV_VALIDATION=1`, set in CI) and DXC run.
// HLSL: = Light_intersect_0(getLight_0(
// SPV: OpEntryPoint
// DXIL: define void @computeMain()

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Thread 0: B hits a light with value 0, which never intersects.
    // Thread 1: B intersects at t = 5; the output is the radiance 5 * 2.
    // Thread 2: A intersects at t = length(origin) = 2; the output is the radiance 7 * 2.
    // Thread 3: B hits a light with value 0, which never intersects.
    float light = getLight(tid.x).radiance();
    float state = 0.0;
    Handler handler = {};
    IGeometry geometry = createDynamicObject<IGeometry>(geometryTypes[tid.x], 0u);
    geometry.intersectAndAccept(float3(tid), handler, state);
    outputBuffer[tid.x] = state > 0.0 ? light : 0.0;
    // CHECK: type: float
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 10.0
    // CHECK-NEXT: 14.0
    // CHECK-NEXT: 0.0
}
