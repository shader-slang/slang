//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct PayloadData
{
    uint value;
}
struct MaterialRecord {}

struct MyTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MyTraceContext;
    typealias Payload = PayloadData;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = MaterialRecord;
}

struct FakeNoIntersection : rt::IIntersectionStage
//CHECK:                        ^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IIntersectionStage' marked 'sealed' in module 'raytracing'
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = FakeNoIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> descriptor;

[shader("raygeneration")]
void main()
{
    PayloadData payload;
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, descriptor, payload);
}
