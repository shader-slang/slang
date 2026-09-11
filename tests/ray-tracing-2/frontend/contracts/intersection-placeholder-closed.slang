//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct PayloadData {}
struct Record {}

struct MyTraceContext : rt::ITraceContext
{
    typealias Payload = PayloadData;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MyTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct FakeNoIntersection : rt::IIntersectionStage
//CHECK:                        ^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IIntersectionStage' marked 'sealed' in module 'raytracing'
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = FakeNoIntersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::HitGroupList<TraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
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
