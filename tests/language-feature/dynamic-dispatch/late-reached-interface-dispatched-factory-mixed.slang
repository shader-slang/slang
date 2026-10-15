// Regression test for dynamic dispatch over one callee whose result type an earlier
// `specializeDynamicInsts` run already rewrote and one whose result type it did not.
//
// `computeMain` calls only `F1.make` directly, so an earlier run rewrites `F1.make`'s declared
// `ILight` result type to a `TaggedUnionType`. `lightIntersect` is reached only in a later run,
// through the `IGeometry.intersectAndAccept` default method, and dispatches `make` over both
// factories, so the dispatcher merges `F1.make`'s rewritten result type with `F2.make`'s
// tagged-union info. If `makeInfoForConcreteType` wrapped the rewritten type as a payload, the
// merge would fail with `Incompatible propagation infos during union`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1 -conformance F1:IFactory=0 -conformance F2:IFactory=1
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1 -conformance F1:IFactory=0 -conformance F2:IFactory=1
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry computeMain -stage compute -conformance Light:ILight=0 -conformance SurfaceLight:ILight=1 -conformance A:IGeometry=0 -conformance B:IGeometry=1 -conformance F1:IFactory=0 -conformance F2:IFactory=1

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

interface IFactory
{
    ILight make(uint id);
}

struct F1 : IFactory
{
    ILight make(uint id)
    {
        LightData data = { lightTypes[id], lightValues[id] };
        return createDynamicObject<ILight>(data.type, data);
    }
}

struct F2 : IFactory
{
    ILight make(uint id)
    {
        SurfaceLight s = { { 1u, lightValues[id] } };
        return s;
    }
}

bool lightIntersect(float3 origin, out float t)
{
    uint id = uint(origin.x);
    IFactory factory = createDynamicObject<IFactory>(lightTypes[id], 0u);
    ILight light = factory.make(id);
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

//TEST_INPUT: type_conformance F1:IFactory = 0
//TEST_INPUT: type_conformance F2:IFactory = 1

// The concrete `intersect` methods must receive the unpacked `AnyValue` payload, not the tagged
// union. The SPIR-V and DXIL runs only match the entry point, so they catch ill-typed output only
// when SPIR-V validation (`SLANG_RUN_SPIRV_VALIDATION=1`, set in CI) and DXC run.
// HLSL-DAG: {{[^A-Za-z]}}Light_intersect_0(unpackAnyValue8_{{[0-9]+}}(
// HLSL-DAG: SurfaceLight_intersect_0(unpackAnyValue8_{{[0-9]+}}(
// SPV: OpEntryPoint
// DXIL: define void @computeMain()

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    F1 f1 = {};
    float light = f1.make(tid.x).radiance();
    outputBuffer[tid.x] = occluded(tid.x, float3(tid)) ? light : 0.0;
    // CHECK: type: float
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 10.0
    // CHECK-NEXT: 14.0
    // CHECK-NEXT: 0.0
}
