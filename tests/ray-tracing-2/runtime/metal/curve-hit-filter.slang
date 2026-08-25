//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

// METAL: intersection_function_table<metal::raytracing::instancing, metal::raytracing::curve_data>
// METAL: assume_geometry_type(metal::raytracing::geometry_type::curve)
// METAL: {{\[\[}}intersection(curve, metal::raytracing::instancing, metal::raytracing::curve_data){{\]\]}}
// METAL-SAME: float curveParameter_{{[0-9]+}} {{\[\[}}curve_parameter{{\]\]}}

import slang.raytracing;

struct RuntimeResult
{
    uint stage;
    uint anyHitCount;
    uint parameterValid;
}

struct RuntimePayload
{
    uint stage;
    uint anyHitCount;
    uint parameterValid;
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
    typealias Primitive = rt::CurvePrimitive;
    typealias Record = void;
}

struct RuntimeAnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input)
    {
        input.payload.anyHitCount++;
        input.payload.parameterValid =
            input.curve.parameter >= 0.0 && input.curve.parameter <= 1.0;
    }
}

struct RuntimeClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.stage = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Slot = rt::HitGroupSlot<0>;
    typealias Context = HitContext;
    typealias ClosestHit = RuntimeClosestHit;
    typealias AnyHit = RuntimeAnyHit;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = RuntimeTraceContext;
    typealias Record = void;
}

struct RuntimeMiss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input)
    {
        input.payload.stage = 2;
    }
}

struct MissGroup : rt::IMissGroup
{
    typealias Slot = rt::MissSlot<0>;
    typealias Context = MissContext;
    typealias Miss = RuntimeMiss;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = RuntimeTraceContext;
    typealias HitGroups = rt::HitGroupList<RuntimeTraceContext, HitGroup>;
    typealias MissGroups = rt::MissGroupList<RuntimeTraceContext, MissGroup>;
    typealias CallableGroups = rt::NoCallableGroups<RuntimeTraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;
RWStructuredBuffer<RuntimeResult> results;

[shader("raygeneration")]
void main()
{
    uint rayIndex = DispatchRaysIndex().x;

    rt::RayTraversalDesc desc = {};
    desc.ray.origin = rayIndex == 0 ? float3(0.0, 0.0, 0.0) : float3(2.0, 2.0, 0.0);
    desc.ray.direction = float3(0.0, 0.0, 1.0);
    desc.ray.tMin = 0.001;
    desc.ray.tMax = 100.0;
    desc.rayFlags = RAY_FLAG_NONE;
    desc.instanceMask = 0xff;

    RuntimePayload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);

    results[rayIndex].stage = payload.stage;
    results[rayIndex].anyHitCount = payload.anyHitCount;
    results[rayIndex].parameterValid = payload.parameterValid;
}
