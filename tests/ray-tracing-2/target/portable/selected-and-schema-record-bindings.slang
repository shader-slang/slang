//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry FirstMiss -stage miss -entry raygenMain -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry FirstMiss -stage miss -entry raygenMain -stage raygeneration -target dxil -profile sm_6_5

import slang.raytracing;

struct Payload
{
    uint value;
}

struct SelectedRecord
{
    uint selected;
}

struct SchemaRecord
{
    uint2 schema;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct SelectedContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::SelectedRecord;
}

struct SchemaContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::SchemaRecord;
}

// This stage is selected directly on the command line, so parameter binding reserves the first
// structural Record register for its entry-point layout.
struct FirstMiss : rt::IMissShader
{
    typealias Context = SelectedContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.selected;
    }
}

// This stage is reachable only through Schema. Its synthesized adapter must continue allocating
// after the directly selected entry instead of reusing b0.
struct SecondMiss : rt::IMissShader
{
    typealias Context = SchemaContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.schema.x;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<SecondMiss>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void raygenMain()
{
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
}

// The selected FirstMiss adapter owns b0. The schema-only SecondMiss adapter begins at the selected
// binding count and therefore owns b1 in the same compiler-reserved space. Reusing b0 would make
// DXC reject the combined library because the two registers have different element types.
// HLSL-DAG: cbuffer record_0 : register(b0, space2)
// HLSL-DAG: SelectedRecord_0 record_0;
// HLSL-DAG: cbuffer record_{{[0-9]+}} : register(b1, space2)
// HLSL-DAG: SchemaRecord_0 record_{{[0-9]+}};

// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb0{{.*}}space2
// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb1{{.*}}space2
