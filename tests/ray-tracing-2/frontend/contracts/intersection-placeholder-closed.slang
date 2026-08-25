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

struct FakeNoIntersection : rt::IIntersectionStage<HitContext> {}
//CHECK:                        ^^^^^^^^^^^^^^^^^^ declaration not accessible
//CHECK:                        ^^^^^^^^^^^^^^^^^^ 'IIntersectionStage' is not accessible from the current context.

struct HitGroup : rt::IHitGroup
//CHECK:              ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:              ^^^^^^^^^ type argument 'HitGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = FakeNoIntersection;
}

struct Layout : rt::ITraceProgramLayout
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::HitGroupList<TraceContext, HitGroup>;
    typealias MissGroups = rt::NoMissGroups<TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Layout> descriptor;

[shader("raygeneration")]
void main()
{
    PayloadData payload;
    rt::RayTracer<Layout> tracer;
    tracer.trace({}, scene, descriptor, payload);
}
