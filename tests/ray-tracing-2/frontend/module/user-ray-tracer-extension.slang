//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage compute -dump-ir -no-codegen

import slang.raytracing;

struct MyPayload {}

struct MyTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MySchema : rt::ITraceProgramSchema
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

RWStructuredBuffer<uint> output;

public extension<Schema> rt::RayTracer<Schema>
    where Schema : rt::ITraceProgramSchema
{
    public void trace(uint value) { output[0] = value; }
}

[numthreads(1, 1, 1)]
void main()
{
    rt::RayTracer<MySchema> tracer;
    tracer.trace(42);
}

// IR: store
// IR-NOT: structuralRayTracingTrace
