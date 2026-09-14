//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct TestPayload
{
    uint value;
}

struct MotionTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MotionTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        input.payload.value = 1;
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = TestClosestHit;
    typealias AnyHit = rt::NoAnyHit<HitContext>;
    typealias Intersection = rt::NoIntersection<HitContext>;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = MotionTraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct SceneParameters
{
    rt::AccelerationStructure scene;
}

ParameterBlock<SceneParameters> sceneParameters;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    desc.time = 0.25;
    TestPayload payload = {};
    rt::RayTracer<Schema> tracer;

    // The resource deliberately lives in a nested parameter-block field. The Metal declaration
    // must preserve the trace context's instance-motion ABI on the storage field, not only on the
    // temporary value passed to `trace`.
    tracer.trace(desc, sceneParameters.scene, program, payload);
}

// METAL: struct SceneParameters_default_0
// METAL: metal::raytracing::acceleration_structure<metal::raytracing::instancing, metal::raytracing::instance_motion> scene_0;
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::instance_motion> _slang_intersector;
