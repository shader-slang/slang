//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry ScalarMiss -stage miss -entry VectorMiss -stage miss -target spirv-assembly -emit-spirv-directly

// A standalone stage is linked without a trace-schema operation. Its front-end entry-point
// metadata must therefore carry the complete program's payload assignment through specialization.
// Scalar and vector payloads also verify that linking preserves the already-lowered IR type instead
// of trying to reconstruct a nominal declaration for it.

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ScalarMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

struct VectorMissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = float2;
    typealias Record = void;
}

struct ScalarMiss : rt::IMissShader
{
    typealias Context = ScalarMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload += 1;
    }
}

struct VectorMiss : rt::IMissShader
{
    typealias Context = VectorMissContext;

    void invoke(rt::MissInput<Context> input)
    {
        input.payload += float2(1.0);
    }
}

// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "ScalarMiss"
// SPIRV-DAG: OpEntryPoint MissKHR {{.*}} "VectorMiss"
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 0
// SPIRV-DAG: OpVariable {{.*}} IncomingRayPayloadKHR {{.*}}Location 1
