//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

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

[require(anyhit)]
void invalidAnyHitHelper(rt::ClosestHitInput<TestHitContext> input)
//CHECK:                     ^^^^^^^^^^^^^^^ ray-tracing stage input used by the wrong stage
//CHECK:                     ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' requires logical stage 'closesthit', but function 'invalidAnyHitHelper' is not restricted to that stage
{
    IgnoreHit();
}

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::ClosestHitInput<TestHitContext> input) { invalidAnyHitHelper(input); }
}

// An attributed entry point already has a concrete native stage. It must not inherit the logical
// stage of its first structural input parameter as an ordinary unannotated helper would.
[shader("compute")]
[numthreads(1, 1, 1)]
void invalidComputeEntry(rt::ClosestHitInput<TestHitContext> input)
//CHECK:                     ^^^^^^^^^^^^^^^ ray-tracing stage input used by the wrong stage
//CHECK:                     ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' requires logical stage 'closesthit', but function 'invalidComputeEntry' is not restricted to that stage
{
}
