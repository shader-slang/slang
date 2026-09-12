//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule

import slang.raytracing;

struct PayloadA
{
    uint value;
}
struct PayloadB
{
    uint value;
}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContextA : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadA;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ::Record;
}

struct HitContextB : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadB;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ::Record;
}

struct HitGroupA0 : rt::IHitGroup
{
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct HitGroupB0 : rt::IHitGroup
{
    typealias Context = HitContextB;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct HitGroupA1 : rt::IHitGroup
{
    typealias Context = HitContextA;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
}

struct MissContextA : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadA;
    typealias Record = ::Record;
}

struct MissContextB : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = PayloadB;
    typealias Record = ::Record;
}

struct MissA0 : rt::IMissShader
{
    typealias Context = MissContextA;
    void invoke(rt::MissInput<Context> input) {}
}

struct MissB0 : rt::IMissShader
{
    typealias Context = MissContextB;
    void invoke(rt::MissInput<Context> input) {}
}

struct MissA1 : rt::IMissShader
{
    typealias Context = MissContextA;
    void invoke(rt::MissInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroupA0, HitGroupB0, HitGroupA1>;
    typealias MissShaders = rt::MissShaderList<MissA0, MissB0, MissA1>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    rt::RayTraversalDesc desc = {};
    PayloadA payload = {};
    tracer.trace(desc, scene, program, payload);
}

// Function indices are dense within a payload partition. Hit and miss sections use independent
// index spaces, so PayloadA and PayloadB both start at zero in each section.
// IR-DAG: [structuralRayTracingHitGroupInfo(%HitGroupA0_, "HitGroupA0", 0 : Int
// IR-DAG: [structuralRayTracingHitGroupInfo(%HitGroupB0_, "HitGroupB0", 0 : Int
// IR-DAG: [structuralRayTracingHitGroupInfo(%HitGroupA1_, "HitGroupA1", 1 : Int
// IR-DAG: [structuralRayTracingMissShaderInfo(0 : Int, %MissContextA{{.*}}%MissA0_,
// IR-DAG: [structuralRayTracingMissShaderInfo(0 : Int, %MissContextB{{.*}}%MissB0_,
// IR-DAG: [structuralRayTracingMissShaderInfo(1 : Int, %MissContextA{{.*}}%MissA1_,
