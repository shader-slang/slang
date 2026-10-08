//TEST:SIMPLE: -experimental-feature -entry AnyHit -target hlsl -no-codegen

import slang.raytracing;

// The helper dispatches a callable only in its closest-hit branch. Capability inference preserves
// the stage switch, so sharing this helper with an any-hit stage is valid. A source call graph that
// ignores the switch would incorrectly reject the any-hit entry point.

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
    __stage_switch
    {
    case closesthit:
        rt::RayTracer<TestSchema> tracer;
        TestCallableData data = {0};
        tracer.callShader<TestCallableContext>(0, program, data);
        break;
    default:
        break;
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::AnyHitInput<TestHitContext> input)
    {
        callCallableFromHelper();
    }
}

struct TestCallable : rt::ICallableShader
{
    typealias Context = TestCallableContext;
    void invoke(rt::CallableInput<TestCallableContext> input) {}
}

struct TestHitGroup : rt::IHitGroup
{
    typealias Context = TestHitContext;
    typealias ClosestHit = rt::NoClosestHit<TestHitContext>;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = rt::NoIntersection<TestHitContext>;
}

struct TestSchema : rt::ITraceProgramSchema
{
    typealias TraceContext = TestTraceContext;
    typealias HitGroups = rt::HitGroupList<TestHitGroup>;
    typealias MissShaders = rt::EmptyMissShaderList;
    typealias CallableShaders = rt::CallableShaderList<TestCallable>;
}

rt::TraceProgramDescriptor<TestSchema> program;
