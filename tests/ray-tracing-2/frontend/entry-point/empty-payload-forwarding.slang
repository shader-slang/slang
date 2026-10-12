//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry NativeNoMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry NativePrimitiveMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry NativeInstanceMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry NativeCombinedMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry MultilevelNoMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry MultilevelPrimitiveMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry MultilevelInstanceMotionMiss -target metal -profile metallib_2_4
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -entry MultilevelCombinedMotionMiss -target metal -profile metallib_2_4

// Every no-payload overload must forward to the actual trace operation. These checks need no
// Metal runtime: code generation must reject the unfinished operation reached through the wrapper.
// Structural miss entries use the supported Metal stage contract; native Metal ray-generation
// functions are not supported. The missing-entry-adapter error alone does not satisfy this check:
// if a wrapper loses its forwarding call, the trace-operation diagnostic must disappear.
// CHECK: structural ray-tracing operation 'trace' reachable from this entry point must be lowered

import slang.raytracing;

struct TraceContext<A : rt::IAccelerationStructure, M : rt::IRayMotion> : rt::ITraceContext
{
    typealias AccelerationStructure = A;
    typealias Motion = M;
}

struct Schema<A : rt::IAccelerationStructure, M : rt::IRayMotion> : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext<A, M>;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

struct EmptyPayload
{}

struct MissContext<T : rt::ITraceContext> : rt::IPayloadContext
{
    typealias TraceContext = T;
    typealias Payload = EmptyPayload;
    typealias Record = void;
}

typealias NativeNoMotion = Schema<rt::AccelerationStructure, rt::NoMotion>;
typealias NativePrimitiveMotion = Schema<rt::AccelerationStructure, rt::PrimitiveMotion>;
typealias NativeInstanceMotion = Schema<rt::AccelerationStructure, rt::InstanceMotion>;
typealias NativeCombinedMotion = Schema<rt::AccelerationStructure, rt::PrimitiveAndInstanceMotion>;
typealias MultilevelNoMotion = Schema<rt::MultiLevelAccelerationStructure<3>, rt::NoMotion>;
typealias MultilevelPrimitiveMotion =
    Schema<rt::MultiLevelAccelerationStructure<3>, rt::PrimitiveMotion>;
typealias MultilevelInstanceMotion =
    Schema<rt::MultiLevelAccelerationStructure<3>, rt::InstanceMotion>;
typealias MultilevelCombinedMotion =
    Schema<rt::MultiLevelAccelerationStructure<3>, rt::PrimitiveAndInstanceMotion>;

rt::AccelerationStructure nativeScene;
rt::MultiLevelAccelerationStructure<3> multilevelScene;
rt::TraceProgramDescriptor<NativeNoMotion> nativeNoMotionProgram;
rt::TraceProgramDescriptor<NativePrimitiveMotion> nativePrimitiveMotionProgram;
rt::TraceProgramDescriptor<NativeInstanceMotion> nativeInstanceMotionProgram;
rt::TraceProgramDescriptor<NativeCombinedMotion> nativeCombinedMotionProgram;
rt::TraceProgramDescriptor<MultilevelNoMotion> multilevelNoMotionProgram;
rt::TraceProgramDescriptor<MultilevelPrimitiveMotion> multilevelPrimitiveMotionProgram;
rt::TraceProgramDescriptor<MultilevelInstanceMotion> multilevelInstanceMotionProgram;
rt::TraceProgramDescriptor<MultilevelCombinedMotion> multilevelCombinedMotionProgram;

struct NativeNoMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<NativeNoMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<NativeNoMotion> tracer;
        tracer.trace({}, nativeScene, nativeNoMotionProgram);
    }
}

struct NativePrimitiveMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<NativePrimitiveMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<NativePrimitiveMotion> tracer;
        tracer.trace({}, nativeScene, nativePrimitiveMotionProgram);
    }
}

struct NativeInstanceMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<NativeInstanceMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<NativeInstanceMotion> tracer;
        tracer.trace({}, nativeScene, nativeInstanceMotionProgram);
    }
}

struct NativeCombinedMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<NativeCombinedMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<NativeCombinedMotion> tracer;
        tracer.trace({}, nativeScene, nativeCombinedMotionProgram);
    }
}

struct MultilevelNoMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<MultilevelNoMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<MultilevelNoMotion> tracer;
        tracer.trace({}, multilevelScene, multilevelNoMotionProgram);
    }
}

struct MultilevelPrimitiveMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<MultilevelPrimitiveMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<MultilevelPrimitiveMotion> tracer;
        tracer.trace({}, multilevelScene, multilevelPrimitiveMotionProgram);
    }
}

struct MultilevelInstanceMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<MultilevelInstanceMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<MultilevelInstanceMotion> tracer;
        tracer.trace({}, multilevelScene, multilevelInstanceMotionProgram);
    }
}

struct MultilevelCombinedMotionMiss : rt::IMissShader
{
    typealias Context = MissContext<MultilevelCombinedMotion.TraceContext>;

    void invoke(rt::MissInput<Context> input)
    {
        rt::RayTracer<MultilevelCombinedMotion> tracer;
        tracer.trace({}, multilevelScene, multilevelCombinedMotionProgram);
    }
}
