//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=NoMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=InstanceMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=NoMotion -DTEST_MULTILEVEL
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveMotion -DTEST_MULTILEVEL
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=InstanceMotion -DTEST_MULTILEVEL
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion -DTEST_MULTILEVEL

// A generic helper should be able to trace the acceleration structure chosen by its caller.
// Both trace overloads must resolve while A is still generic; the helper does not need to
// constrain A to the native or multilevel handle family. Test every supported motion mode.
import slang.raytracing.portable;

struct Payload
{
    uint value;
}

struct TraceContext<A : rt::IAccelerationStructure> : rt::ITraceContext
{
    typealias AccelerationStructure = A;
    typealias Motion = rt::TEST_MOTION;
}

struct Schema<A : rt::IAccelerationStructure> : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext<A>;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

void traceRays<A : rt::IAccelerationStructure>(
    A scene,
    rt::TraceProgramDescriptor<Schema<A>> descriptor,
    inout Payload payload)
{
    rt::RayTracer<Schema<A>> tracer;
    tracer.trace({}, scene, descriptor, payload);
    tracer.trace({}, scene, descriptor);
}

#ifdef TEST_MULTILEVEL
typealias Scene = rt::MultiLevelAccelerationStructure<3>;
#else
typealias Scene = rt::AccelerationStructure;
#endif

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext<Scene>;
    typealias Payload = ::Payload;
    typealias Record = void;
}

Scene scene;
rt::TraceProgramDescriptor<Schema<Scene>> descriptor;

struct TestMiss : rt::IMissShader
{
    typealias Context = MissContext;

    static void invoke(rt::MissInput<Context> input)
    {
        traceRays(scene, descriptor, input.payload);
    }
}
