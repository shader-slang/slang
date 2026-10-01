//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry raygenMain -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry raygenMain -stage raygeneration -target dxil -profile sm_6_5

import slang.raytracing;

struct Payload
{
    uint value;
}

struct FirstRecord
{
    uint first;
}

struct SecondRecord
{
    uint2 second;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct FirstMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::FirstRecord;
}

struct SecondMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::SecondRecord;
}

struct FirstMiss : rt::IMissShader
{
    typealias Context = FirstMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.first;
    }
}

struct SecondMiss : rt::IMissShader
{
    typealias Context = SecondMissContext;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record.second.x;
    }
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<FirstMiss, SecondMiss>;
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

// The schema's public resources occupy the preceding spaces. Both schema-only miss adapters
// therefore receive distinct cbuffer registers in one fresh compiler-owned space. Reusing b0 for
// both Record types makes DXC reject the combined library before FileCheck runs.
// The checks also pin each type to its register. This catches assigning bindings while iterating a
// hash set: such code still emits {b0, b1}, but can nondeterministically swap the stage mapping.
// HLSL: cbuffer record_0 : register(b0, space2)
// HLSL: SecondRecord_0 record_0;
// HLSL: cbuffer record_1 : register(b1, space2)
// HLSL: FirstRecord_0 record_1;

// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb0{{.*}}space2
// DXIL-DAG: ; record_{{[0-9]+}}{{.*}}cbuffer{{.*}}cb1{{.*}}space2
