//TEST:SIMPLE(filecheck=RECURSIVE): -experimental-feature -entry RecursiveClosestHit -stage closesthit -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=HIGHER_ORDER): -experimental-feature -entry HigherOrderRaygen -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL_RECURSIVE): -experimental-feature -entry RecursiveClosestHit -stage closesthit -target dxil -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL_HIGHER_ORDER): -experimental-feature -entry HigherOrderRaygen -stage raygeneration -target dxil -profile sm_6_5

import slang.raytracing;

struct Payload
{
    uint value;
}

struct Record
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct CallerContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct RecordContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::Record;
}

struct RecordMiss : rt::IMissShader
{
    typealias Context = RecordContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.value;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<RecordMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

void traceWithRecordSchema()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

void invokeFunction(functype()->void function)
{
    function();
}

struct RecursiveClosestHit : rt::IClosestHitShader
{
    typealias Context = CallerContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        traceWithRecordSchema();
    }
}

[shader("raygeneration")]
void HigherOrderRaygen()
{
    // The structural trace is hidden behind a function-valued parameter until higher-order
    // specialization. Binding reservation must still agree with the adapters synthesized later.
    invokeFunction(traceWithRecordSchema);
}

// The selected closest-hit stage has Record = void, while the recursively traced schema's miss
// stage reads a non-void Record. That synthesized miss adapter still needs a local cbuffer binding.
// RECURSIVE: cbuffer record{{.*}} : register(b0, space2)
// RECURSIVE: [shader("closesthit")]void RecursiveClosestHit(
// RECURSIVE: [shader("miss")]void RecordMiss(

// HIGHER_ORDER: cbuffer record{{.*}} : register(b0, space2)
// HIGHER_ORDER: [shader("raygeneration")]void HigherOrderRaygen()
// HIGHER_ORDER: [shader("miss")]void RecordMiss(

// DXIL_RECURSIVE: record{{.*}} cbuffer{{.*}} cb0,space2
// DXIL_RECURSIVE: RecursiveClosestHit
// DXIL_RECURSIVE: RecordMiss

// DXIL_HIGHER_ORDER: record{{.*}} cbuffer{{.*}} cb0,space2
// DXIL_HIGHER_ORDER: HigherOrderRaygen
// DXIL_HIGHER_ORDER: RecordMiss
