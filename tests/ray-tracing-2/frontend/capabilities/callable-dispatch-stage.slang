// Callable dispatch is forbidden in any-hit and intersection stages, even through a helper.
// Re-enable when stage-input parameter types propagate their capability requirements:
// https://github.com/shader-slang/slang/issues/13579
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen
//CHECK: error[E361
//CHECK: error[E361

import slang.raytracing.portable;

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

// Both stages can use this context; intersection requires a procedural primitive.
struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = uint;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

struct TestCallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = uint;
    typealias Record = void;
}

// Checking callShader's stage restriction does not require assembling shader lists.
struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::EmptyHitGroupList;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::EmptyCallableShaderList;
}

rt::TraceProgramDescriptor<TestSchema> program;

void callCallableFromHelper()
{
    rt::RayTracer<TestSchema> tracer;
    uint data = 0;
    tracer.callShader<TestCallableContext>(0, program, data);
}

struct BadAnyHit : rt::IAnyHitShader
{
    typealias Context = TestHitContext;
    static void invoke(rt::AnyHitInput<TestHitContext> input)
    {
        callCallableFromHelper();
    }
}

struct BadIntersection : rt::IIntersectionShader
{
    typealias Context = TestHitContext;
    static void invoke(rt::IntersectionInput<TestHitContext> input)
    {
        callCallableFromHelper();
    }
}
