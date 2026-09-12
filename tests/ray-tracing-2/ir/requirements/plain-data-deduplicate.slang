//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct InvalidPayload
{
    Texture2D texture;
}

struct GoodPayload
{
    uint value;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext<PayloadType> : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = PayloadType;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct GoodHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<GoodPayload>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

// These are distinct SBT entries that deliberately share InvalidPayload. The
// plain-data validator should diagnose that payload type only once.
struct InvalidHitGroupA : rt::IHitGroup
{
    typealias Context = TestHitContext<InvalidPayload>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct InvalidHitGroupB : rt::IHitGroup
{
    typealias Context = TestHitContext<InvalidPayload>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct InvalidHitGroupC : rt::IHitGroup
{
    typealias Context = TestHitContext<InvalidPayload>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<
        GoodHitGroup,
        InvalidHitGroupA,
        InvalidHitGroupB,
        InvalidHitGroupC>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<TestSchema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<TestSchema> tracer;
    GoodPayload payload = {};
    tracer.trace({}, scene, program, payload);
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'InvalidPayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
}
