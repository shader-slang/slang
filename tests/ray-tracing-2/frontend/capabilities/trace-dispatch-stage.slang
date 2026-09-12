//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry BadAnyHit -stage anyhit -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry BadAnyHit -stage anyhit -target metal

import slang.raytracing;

struct PayloadData
{
    float value;
}

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
    typealias Record = uint;
}

struct BadAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input)
//CHECK: ^^^^^^ dependencies not compatible on stage
//CHECK: ^^^^^^ 'invoke' requires support for stage 'anyhit', but stage is unsupported.
    {
        rt::RayTracer<Schema> tracer;
        tracer.trace({}, scene, descriptor, input.payload);
//CHECK:                                          ^^^^^^^ conflicting capability requirement
//CHECK:                                          ^^^^^^^ 'payload' requires capability
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<HitContext>;
    typealias AnyHit = BadAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
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
