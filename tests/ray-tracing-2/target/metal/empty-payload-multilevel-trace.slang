//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target metal -capability metallib_3_1 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal -capability metallib_3_1

import slang.raytracing;

struct EmptyPayload
{}

// This type has source-level fields and is therefore a valid explicit payload. Its direct field
// and its fixed array are nevertheless removed by physical empty-type legalization, so Metal's
// internal ray-data carrier must still remain concrete.
struct NestedEmptyPayload
{
    EmptyPayload value;
    EmptyPayload values[2];
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::MultiLevelAccelerationStructure<3>;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct NestedMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = NestedEmptyPayload;
    typealias Record = void;
}

struct NestedMiss : rt::IMissShader
{
    typealias Context = NestedMissContext;
    void invoke(rt::MissInput<Context> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

struct NestedSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<NestedMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::MultiLevelAccelerationStructure<3> scene;
rt::TraceProgramDescriptor<Schema> program;
rt::TraceProgramDescriptor<NestedSchema> nestedProgram;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program);

    NestedEmptyPayload nestedPayload = {};
    rt::RayTracer<NestedSchema> nestedTracer;
    nestedTracer.trace(desc, scene, nestedProgram, nestedPayload);
}

// The no-payload multilevel overload still carries its inferred level count. Pairing it with the
// payload-taking fallback must preserve that ordinary generic argument and its checked witness.
// IR: structuralRayTracingTrace({{.*}}, %Schema, "Schema", %TraceContext, %EmptyPayload,
// IR: structuralRayTracingTrace({{.*}}, %NestedSchema, "NestedSchema", %TraceContext,
// IR-SAME: %NestedEmptyPayload,
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::max_levels<3>> _slang_intersector;
// METAL: metal::raytracing::intersection_result<metal::raytracing::instancing, metal::raytracing::max_levels<3>> _slang_result
// METAL: metal::raytracing::intersector<metal::raytracing::instancing, metal::raytracing::max_levels<3>> _slang_intersector;
// METAL: metal::raytracing::intersection_result<metal::raytracing::instancing, metal::raytracing::max_levels<3>> _slang_result
