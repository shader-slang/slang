//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -target hlsl -profile sm_6_5 -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -target metal -entry main -stage raygeneration

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct SharedClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) { input.payload.value += 1; }
}

struct SharedHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SharedClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

// A second logical record in the first schema deliberately reuses the same concrete stage.
struct RepeatedStageHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = SharedClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct FirstSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<SharedHitGroup, RepeatedStageHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct SecondSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<SharedHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<FirstSchema> firstProgram;
rt::TraceProgramDescriptor<SecondSchema> secondProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<FirstSchema> firstTracer;
    firstTracer.trace(desc, scene, firstProgram, payload);
    rt::RayTracer<SecondSchema> secondTracer;
    secondTracer.trace(desc, scene, secondProgram, payload);
}

// Multiple records in FirstSchema and another record in SecondSchema all reference the same
// semantic stage and request the same portable symbol. The D3D/Vulkan path therefore shares one
// native entry-point adapter across both forms of reuse.
// HLSL-COUNT-1: [shader("closesthit")]void SharedClosestHit(

// Metal must keep the schema-specific ray-data parameter type in the physical signature. It emits
// one adapter per logical record, while all adapters still run the one SharedClosestHit
// implementation.
// METAL-COUNT-2: {{\[\[visible\]\]}} void {{.*}}(FirstSchema_rayData_0 thread*
// METAL-COUNT-1: {{\[\[visible\]\]}} void {{.*}}(SecondSchema_rayData_0 thread*
