//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -ignore-capabilities -entry main -stage compute -target hlsl
// HLSL: void main()

import slang.raytracing.portable;

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
    static void invoke(in rt::MissInput<Context> input) {}
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
    // An ordinary generic helper accepts both a schema and a concrete stage struct.
    static uint helper<T>() { return 1; }
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = rt::RayTracer<TestSchema>.helper<TestSchema>();
    output[1] = rt::RayTracer<TestSchema>.helper<uint>();
    output[2] = rt::RayTracer<TestSchema>.helper<TestMiss>();
}
