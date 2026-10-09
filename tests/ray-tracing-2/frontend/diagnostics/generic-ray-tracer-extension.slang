//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STAGE): -experimental-feature -ignore-capabilities -DINVALID_STAGE -entry main -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SCHEMA): -experimental-feature -ignore-capabilities -DINVALID_SCHEMA -entry main -stage compute -no-codegen
// HLSL: void main()

import slang.raytracing;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Record = void;
    typealias Payload = uint;
}

struct TestMiss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(in rt::MissInput<Context> input) {}
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

extension<S> rt::RayTracer<S> where S : rt::ITraceProgramSchema
{
    // S belongs to the compiler-recognized RayTracer container. T belongs to an ordinary user
    // method, which cannot consume structural types merely because it extends that container.
    static uint helper<T>() { return 1; }
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
#if defined(INVALID_STAGE)
    output[0] = rt::RayTracer<TestSchema>.helper<TestMiss>();
// STAGE: structural ray-tracing stage type 'TestMiss' has no runtime representation
#elif defined(INVALID_SCHEMA)
    output[0] = rt::RayTracer<TestSchema>.helper<TestSchema>();
// SCHEMA: structural ray-tracing metadata type 'TestSchema' has no runtime representation
#else
    // The same call with an ordinary T remains valid: the enclosing TestSchema substitution is
    // still allowed, so removing the entire container exemption is not a correct fix.
    output[0] = rt::RayTracer<TestSchema>.helper<uint>();
#endif
}
