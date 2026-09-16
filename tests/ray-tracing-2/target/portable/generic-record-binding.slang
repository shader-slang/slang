//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry raygenMain -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=DXIL): -experimental-feature -entry raygenMain -stage raygeneration -target dxil -profile sm_6_5

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Payload<T>
{
    T value;
}

struct MissContext<T> : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload<T>;
    typealias Record = T;
}

struct GenericMiss<T> : rt::IMissShader
{
    typealias Context = MissContext<T>;
    void invoke(rt::MissInput<Context> input)
    {
        input.payload.value = input.record;
    }
}

struct GenericSchema<T> : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<GenericMiss<T>>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<GenericSchema<uint>> traceProgram;

void traceValue<T>(
    rt::TraceProgramDescriptor<GenericSchema<T>> program,
    inout Payload<T> payload)
{
    rt::RayTraversalDesc desc = {};
    desc.ray.direction = float3(0.0f, 0.0f, 1.0f);
    desc.ray.tMax = 1.0f;
    desc.instanceMask = 0xff;

    rt::RayTracer<GenericSchema<T>> tracer;
    tracer.trace(desc, scene, program, payload);
}

[shader("raygeneration")]
void raygenMain()
{
    Payload<uint> payload = {};
    traceValue(traceProgram, payload);
}

// The `input.record` access in `GenericMiss<T>` remains a compiler-owned operation over symbolic
// `T` until the schema is specialized. Binding reservation must follow that use without requiring
// `T` to be a concrete type, and the generated uint adapter must receive an ordinary D3D cbuffer.
// HLSL: cbuffer record_0 : register(b0, space2)
// HLSL: [shader("miss")]void GenericMiss_Sh{{[0-9A-Fa-f]+}}(

// DXIL: ; cbuffer record_0
// DXIL: ; record_0{{.*}}cbuffer{{.*}}cb0{{.*}}space2
