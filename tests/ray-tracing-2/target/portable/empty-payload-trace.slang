//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5 -dump-ir-after specializeModule
//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -profile glsl_460 -emit-spirv-directly

import slang.raytracing;

struct EmptyPayload
{}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
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

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::MissShaderList<Miss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTracer<Schema> tracer;
    rt::RayTraversalDesc desc = {};
    tracer.trace(desc, scene, program);
}

// The hit group and miss shader serve the same nominal empty payload. The compiler deduplicates
// that type and creates the payload storage needed by the ordinary trace operation.
// IR: struct [[EMPTY_PAYLOAD:%EmptyPayload]] : Type
// IR: [[PAYLOAD:%[0-9]+]] : Ptr([[EMPTY_PAYLOAD]]) = var
// IR: [[EMPTY_VALUE:%[0-9]+]] : [[EMPTY_PAYLOAD]] = makeStruct
// IR: store([[PAYLOAD]], [[EMPTY_VALUE]])
// IR: structuralRayTracingTrace({{.*}}, %Schema, "Schema", %TraceContext, [[EMPTY_PAYLOAD]],
// IR-SAME: {{.*}}[[PAYLOAD]], 0 : Int)

// HLSL: TraceRay(
// SPIRV: OpTraceRayKHR
