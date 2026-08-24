//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry main -stage compute -dump-ir -no-codegen

import slang.raytracing;

struct MyPayload {}

struct MyTraceContext : rt::ITraceContext
{
    typealias Payload = MyPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MyProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = MyTraceContext;
    typealias HitGroups = rt::NoHitGroups<MyTraceContext>;
    typealias MissGroups = rt::NoMissGroups<MyTraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<MyTraceContext>;
}

RWStructuredBuffer<uint> output;

public extension<ProgramLayout> rt::RayTracer<ProgramLayout>
    where ProgramLayout : rt::ITraceProgramLayout
{
    public void trace(uint value) { output[0] = value; }
}

[numthreads(1, 1, 1)]
void main()
{
    rt::RayTracer<MyProgramLayout> tracer;
    tracer.trace(42);
}

// IR: store
// IR-NOT: structuralRayTracingTrace
