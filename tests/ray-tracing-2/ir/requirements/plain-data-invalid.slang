//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5

import slang.raytracing;

struct GoodPayload
{
    uint value;
}

struct ResourceLeaf
{
    Texture2D texture;
}

struct ResourcePayload
{
    ResourceLeaf nested;
}

struct PointerLeaf
{
    uint* value;
}

struct PointerRecord
{
    PointerLeaf nested;
}

interface IInterfaceValue
{}

struct InterfacePayload
{
    IInterfaceValue nested;
}

struct AtomicLeaf
{
    Atomic<uint> value;
}

struct AtomicPayload
{
    AtomicLeaf nested;
}

[__NonCopyableType]
struct NonCopyableLeaf
{
    uint value;
}

struct NonCopyableRecord
{
    NonCopyableLeaf nested;
}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext<PayloadType, RecordType> : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = PayloadType;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = RecordType;
}

struct GoodHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<GoodPayload, void>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct ResourcePayloadHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<ResourcePayload, void>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct PointerRecordHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<GoodPayload, PointerRecord>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct InterfacePayloadHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<InterfacePayload, void>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct AtomicPayloadHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<AtomicPayload, void>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct NonCopyableRecordHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<GoodPayload, NonCopyableRecord>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct VoidPayloadHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext<void, void>;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<
        GoodHitGroup,
        ResourcePayloadHitGroup,
        PointerRecordHitGroup,
        InterfacePayloadHitGroup,
        AtomicPayloadHitGroup,
        NonCopyableRecordHitGroup,
        VoidPayloadHitGroup>;
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
//CHECK:        ^ structural ray-tracing ABI data cannot be void
//CHECK:        ^ 'void' cannot be used as structural ray-tracing payload data; use a concrete plain-data type
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'NonCopyableRecord' is used as structural ray-tracing record data but does not have a fixed-size, copyable plain-data representation
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'AtomicPayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'InterfacePayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'PointerRecord' is used as structural ray-tracing record data but does not have a fixed-size, copyable plain-data representation
//CHECK:        ^ structural ray-tracing ABI data is not portable plain data
//CHECK:        ^ type 'ResourcePayload' is used as structural ray-tracing payload data but does not have a fixed-size, copyable plain-data representation
}
