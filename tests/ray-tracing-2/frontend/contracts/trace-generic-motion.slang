//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=InstanceMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target metal -profile metallib_2_4 -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion
//TEST:SIMPLE: -experimental-feature -entry TestMiss -target spirv -capability raytracing_motionblur -no-codegen -DTEST_MOTION=InstanceMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry TestMiss -target hlsl -profile sm_6_5 -no-codegen -DTEST_MOTION=PrimitiveMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry TestMiss -target hlsl -profile sm_6_5 -no-codegen -DTEST_MOTION=InstanceMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry TestMiss -target hlsl -profile sm_6_5 -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry TestMiss -target spirv -capability raytracing_motionblur -no-codegen -DTEST_MOTION=PrimitiveMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -experimental-feature -entry TestMiss -target spirv -capability raytracing_motionblur -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=VERSION): -experimental-feature -entry TestMiss -target metal -profile metallib_2_3 -restrictive-capability-check -no-codegen -DTEST_MOTION=PrimitiveMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=VERSION): -experimental-feature -entry TestMiss -target metal -profile metallib_2_3 -restrictive-capability-check -no-codegen -DTEST_MOTION=InstanceMotion
//DIAGNOSTIC_TEST:SIMPLE(diag=VERSION): -experimental-feature -entry TestMiss -target metal -profile metallib_2_3 -restrictive-capability-check -no-codegen -DTEST_MOTION=PrimitiveAndInstanceMotion

// The helper needs to know only that motion is enabled. Selecting a concrete motion type
// at the call site must still enforce that type's target and Metal version requirements.
// Library-definition notes have no source location, so the target-error runs are non-exhaustive.
import slang.raytracing.portable;

void traceEnabled<Schema : rt::ITraceProgramSchema, Payload>(
    Schema.TraceContext.AccelerationStructure scene,
    rt::TraceProgramDescriptor<Schema> program,
    inout Payload payload)
    where Schema.TraceContext.Motion : rt::IEnabledRayMotion
{
    rt::RayTracer<Schema> tracer;
    tracer.trace({}, scene, program, payload);
    tracer.trace({}, scene, program);
}

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::TEST_MOTION;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = void;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

struct TestMiss : rt::IMissShader
{
    typealias Context = MissContext;

    static void invoke(rt::MissInput<Context> input)
/*CHECK:
                ^^^^^^ unavailable features in entry point
                ^^^^^^ entrypoint 'invoke' uses features that are not available
*/
/*VERSION:
                ^^^^^^ entry point uses capabilities not in specified profile
                ^^^^^^ Missing capabilities are: 'metallib_2_4'
*/
    {
        traceEnabled<Schema, Payload>(scene, program, input.payload);
/*CHECK:
        ^^^^^^^^^^^^ see using of
*/
    }
}
