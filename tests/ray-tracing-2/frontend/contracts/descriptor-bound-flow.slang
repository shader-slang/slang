//TEST:SIMPLE: -experimental-feature -entry main -stage compute -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target cuda
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target metal
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CODEGEN): -experimental-feature -entry main -stage compute -target hlsl -minimum-slang-optimization
//CODEGEN: structural ray-tracing program descriptor must be lowered before target code generation

import slang.raytracing;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
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
