// The unselected method's ClosestHitInput parameter must conflict with its any-hit-only body.
// Re-enable when generic parameter-type requirements propagate correctly:
// https://github.com/shader-slang/slang/issues/13579
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen
//CHECK: error[E361

// Selecting the same method already rejects its any-hit-only body during entry-point checking.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ENTRY): -experimental-feature -entry TestClosestHit -target hlsl -no-codegen
//ENTRY: error[E36107]: unavailable features in entry point

import slang.raytracing.portable;

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

[require(anyhit)]
void invalidAnyHitHelper()
{
    IgnoreHit();
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    static void invoke(rt::ClosestHitInput<TestHitContext> input)
    {
        invalidAnyHitHelper();
    }
}
