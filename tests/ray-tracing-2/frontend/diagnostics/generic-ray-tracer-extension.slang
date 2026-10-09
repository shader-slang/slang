//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=STAGE): -experimental-feature -ignore-capabilities -DINVALID_STAGE -entry main -stage compute -no-codegen
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
    // A schema is an ordinary generic argument. Stage/input arguments remain restricted on
    // ordinary methods because their bodies could construct or store those compiler-supplied types.
    static uint helper<T>() { return 1; }
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
#if defined(INVALID_STAGE)
    output[0] = rt::RayTracer<TestSchema>.helper<TestMiss>();
// STAGE: structural ray-tracing stage type 'TestMiss' has no runtime representation
#else
    output[0] = rt::RayTracer<TestSchema>.helper<TestSchema>();
    output[1] = rt::RayTracer<TestSchema>.helper<uint>();
#endif
}
