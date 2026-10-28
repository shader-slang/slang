// The unselected methods' input types must conflict with their callable-dispatch bodies.
// Re-enable when generic parameter-type requirements propagate correctly:
// https://github.com/shader-slang/slang/issues/13579
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen
//CHECK: error[E361
//CHECK: error[E361

import slang.raytracing;

// Callable dispatch is forbidden in any-hit and intersection stages, including through ordinary
// helpers. Both methods must report an ordinary capability conflict during module checking.

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestProceduralContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = TestRecord;
}

struct TestCallableData
{
    uint value;
}

struct TestCallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestCallableData;
    typealias Record = TestRecord;
}

void callCallableFromHelper()
{
    rt::RayTracer<TestSchema> tracer;
    TestCallableData data = {0};
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
    typealias Context = TestProceduralContext;
    static void invoke(rt::IntersectionInput<TestProceduralContext> input)
    {
        callCallableFromHelper();
    }
}

struct TestCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    static void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<TestHitContext>;
    typealias AnyHit = BadAnyHit;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestProceduralHitGroup : rt::IHitGroup
{
    typealias Context = TestProceduralContext;
    typealias ClosestHit = rt::NoClosestHit<TestProceduralContext>;
    typealias AnyHit = rt::NoAnyHit<TestProceduralContext>;
    typealias Intersection = BadIntersection;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup, TestProceduralHitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::CallableShaderList<TestCallable>;
}

rt::TraceProgramDescriptor<TestSchema> program;
