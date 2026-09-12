//TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

enum MaterialKind
{
    matte,
    mirror,
}

struct NestedValue<T>
{
    T value;
}

struct TestPayload
{
    float3 color;
    float4x4 transform;
    MaterialKind materialKind;
    uint weights[4];
    NestedValue<float2> nested;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    // `void` is the portable representation of an absent shader record.
    typealias Record = void;
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestSchema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestSchema> tracer;
    TestPayload payload = {};
    tracer.trace({}, scene, program, payload);
}

// CHECK: [shader("raygeneration")]void main()
