//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// METAL: metal::visible_function_table<void(Schema_rayData_0 thread*, uchar thread*)> closestHitFunctions_0;
// METAL: ](&rayData_{{[0-9]+}}, (thread uchar*)&kernelContext_0);
// METAL-DAG: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_rayData_0 thread* {{.*}}, uchar thread* kernelContext_{{[0-9]+}})
// METAL-DAG: {{\[\[visible\]\]}} void _slang_structural_rt_{{[0-9a-f]+}}_{{[0-9]+}}(Schema_rayData_0 thread* {{.*}}, uchar thread* kernelContext_{{[0-9]+}})

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Payload = RuntimePayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> output;

struct GlobalClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = output[0];
    }
}

struct PlainClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 2;
    }
}

struct GlobalHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = GlobalClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct PlainHitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = PlainClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<GlobalHitGroup, PlainHitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    output[1] = payload.value;
}
