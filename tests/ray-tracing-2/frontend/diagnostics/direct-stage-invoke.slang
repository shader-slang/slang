//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    void invoke(rt::ClosestHitInput<TestHitContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch

        invoke(input);
//CHECK:^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}

// A stage implementation loaded from a serialized standard module does not recheck its source
// body. Its checked conformance must still identify `invoke` as a compiler-dispatched method.
extension rt::NoClosestHit<TestHitContext>
{
    void illegal(in rt::ClosestHitInput<TestHitContext> input)
    {
        this.invoke(input);
//CHECK:     ^^^^^^ direct invocation of a structural ray-tracing stage
//CHECK:     ^^^^^^ a structural ray-tracing stage method may only be invoked by compiler-generated dispatch
    }
}
