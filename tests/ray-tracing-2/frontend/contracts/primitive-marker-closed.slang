//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry main -stage raygeneration -no-codegen

import slang.raytracing;

struct FakePrimitive : rt::IIntersectionPrimitive
//CHECK:                   ^^^^^^^^^^^^^^^^^^^^^^ cannot inherit from type 'rt.IIntersectionPrimitive' marked 'sealed' in module 'raytracing'
{
    typealias Attributes = float2;
}

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
    typealias Primitive = FakePrimitive;
    typealias Record = MaterialRecord;
}

struct ClosestHitStage : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct HitGroup : rt::IHitGroup
//CHECK:              ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:              ^^^^^^^^^ type argument 'HitGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Context = HitContext;
    typealias ClosestHit = ClosestHitStage;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
//CHECK:                         ^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                         ^^^^^^^^^^^^^^ type argument 'FakePrimitive' does not conform to the required interface 'rt.IBuiltinIntersectionPrimitive'
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
