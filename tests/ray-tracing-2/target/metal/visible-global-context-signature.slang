//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// METAL: metal::visible_function_table<void(ProgramLayout_rayData_0 thread*, uchar thread*)> closestHitFunctions_0;
// METAL: _S{{[0-9]+}}[_slang_hit_slot](&rayData_{{[0-9]+}}, (thread uchar*)&kernelContext_0);
// METAL-DAG: {{\[\[visible\]\]}} void GlobalClosestHit_0(ProgramLayout_rayData_0 thread* {{.*}}, uchar thread* kernelContext_{{[0-9]+}})
// METAL-DAG: {{\[\[visible\]\]}} void PlainClosestHit_0(ProgramLayout_rayData_0 thread* {{.*}}, uchar thread* kernelContext_{{[0-9]+}})

import slang.raytracing;

struct RuntimePayload
{
    uint value;
}

struct RuntimeTraceContext : rt::ITraceContext
{
    typealias Payload = RuntimePayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

RWStructuredBuffer<uint> output;

struct GlobalClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = output[0];
    }
}

struct PlainClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 2;
    }
}

struct GlobalHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = GlobalClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct PlainHitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<1>;
    typealias Context = HitContext;
    typealias ClosestHit = PlainClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, GlobalHitGroup, PlainHitGroup>;
    typealias MissGroups = rt::NoMissGroups<RuntimeTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMax = 100.0;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
    output[1] = payload.value;
}
