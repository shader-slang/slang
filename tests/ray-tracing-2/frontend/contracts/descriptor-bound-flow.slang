//TEST:SIMPLE: -experimental-feature -entry main -stage compute -target hlsl -no-codegen

import slang.raytracing;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::TraceProgramDescriptor<Schema> descriptor;

struct Bindings
{
    rt::TraceProgramDescriptor<Schema> descriptor;
}
ParameterBlock<Bindings> bindings;

T identity<T>(T value)
{
    return value;
}

[numthreads(1, 1, 1)]
void main()
{
    // Opaque means the descriptor has no source-visible representation or public constructor. It
    // remains a value handle, so ordinary generic code may copy one supplied by host binding.
    rt::TraceProgramDescriptor<Schema> copy = identity(descriptor);
    copy = identity(bindings.descriptor);
}
