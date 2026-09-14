//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::InstanceMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = 1;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure firstScene;
rt::AccelerationStructure secondScene;
rt::TraceProgramDescriptor<Schema> program;
uniform bool selectSecond;

[shader("raygeneration")]
void main()
{
    // A motion-qualified acceleration-structure type must reach both arms of the selection. The
    // selected value is represented by a local SSA join after lowering; changing only the final
    // trace operand would leave `firstScene` and `secondScene` with incompatible Metal types.
    rt::AccelerationStructure selectedScene = selectSecond ? secondScene : firstScene;
    rt::RayTraversalDesc desc = {};
    desc.time = 0.25;
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, selectedScene, program, payload);
}

// METAL-DAG: metal::raytracing::acceleration_structure<metal::raytracing::instancing, metal::raytracing::instance_motion> firstScene_0;
// METAL-DAG: metal::raytracing::acceleration_structure<metal::raytracing::instancing, metal::raytracing::instance_motion> secondScene_0;
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::instance_motion> _slang_intersector;
