//TEST:SIMPLE: -experimental-feature -entry main -stage raygeneration -no-codegen

// A structural stage and its caller can use both the public trace wrapper and native TraceRay.
// Calling a tracing function does not introduce a second shader declaration model.
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

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

struct StructuralMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::MissShaderList<StructuralMiss>;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

void traceNative(rt::RayTraversalDesc desc, inout Payload payload)
{
    TraceRay(scene, 0, 0xff, 0, 0, 0, desc.ray, payload);
}

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};

    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    traceNative(desc, payload);
}
