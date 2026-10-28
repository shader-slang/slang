// Native entry-point signatures reject structural inputs independently of capability checking.
//DIAGNOSTIC_TEST:SIMPLE(diag=NATIVE): -experimental-feature -ignore-capabilities -no-codegen
// A helper's explicit anyhit requirement must conflict with its ClosestHitInput parameter.
// Re-enable this helper-only run after https://github.com/shader-slang/slang/issues/13579.
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=HELPER): -experimental-feature -DTEST_HELPER=1 -no-codegen
//HELPER: error[E361

#pragma warning(disable: 38040)

import slang.raytracing;

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

#ifdef TEST_HELPER
[require(anyhit)]
void invalidAnyHitHelper(rt::ClosestHitInput<TestHitContext> input)
{
    IgnoreHit();
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    static void invoke(rt::ClosestHitInput<TestHitContext> input) { invalidAnyHitHelper(input); }
}

#else
// A native entry cannot receive a structural input, regardless of its shader stage.
[shader("compute")]
[numthreads(1, 1, 1)]
void invalidComputeEntry(rt::ClosestHitInput<TestHitContext> input)
//NATIVE:                    ^^^^^^^^^^^^^^^ invalid native entry point signature
//NATIVE:                    ^^^^^^^^^^^^^^^ cannot be used in a native entry point signature
{
}

#endif
