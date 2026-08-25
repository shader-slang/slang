//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct FakePrimitive : rt::IIntersectionPrimitive
//CHECK:                   ^^^^^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IIntersectionPrimitive' marked 'sealed' in module 'raytracing'
{
    typealias Attributes = float2;
}

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
    typealias Primitive = FakePrimitive;
    typealias Record = Record;
}

struct ClosestHitStage : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = ClosestHitStage;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
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
