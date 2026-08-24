//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct TestClosestHit : rt::IClosestHitShader<TestHitContext>
{
    void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct Box<T>
{
    T value;
}

[require(closesthit)]
void invalidStorage(
    TestClosestHit stage,
//CHECK:           ^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:           ^^^^^ structural ray-tracing stage type 'TestClosestHit' has no runtime representation
    rt::ClosestHitInput<TestHitContext> input)
{
    rt::ClosestHitInput<TestHitContext> localInput;
//CHECK:                                ^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                ^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' is compiler-provided and may only be used as a value parameter

    Box<rt::ClosestHitInput<TestHitContext>> wrappedInput;
//CHECK:                                     ^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                     ^^^^^^^^^^^^ ray-tracing stage input type 'Box<rt.ClosestHitInput<TestHitContext>>' is compiler-provided and may only be used as a value parameter

    Box<TestClosestHit> wrappedStage;
//CHECK:                ^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                ^^^^^^^^^^^^ structural ray-tracing stage type 'Box<TestClosestHit>' has no runtime representation

    TestClosestHit();
/*CHECK:
    ^^^^^^^^^^^^^^ construction of a structural ray-tracing type
    ^^^^^^^^^^^^^^ structural ray-tracing type 'TestClosestHit' cannot be constructed by user code
*/

    rt::ClosestHitInput<TestHitContext>();
//CHECK:^^^^^^^^^^^^^^^ construction of a structural ray-tracing type
//CHECK:^^^^^^^^^^^^^^^ structural ray-tracing type 'rt.ClosestHitInput<TestHitContext>' cannot be constructed by user code
}

rt::ClosestHitInput<TestHitContext> invalidResult();
/*CHECK:
    ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
    ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' is compiler-provided and may only be used as a value parameter
*/

[require(closesthit)]
void invalidInterfaceValue(rt::IClosestHitShader<TestHitContext> stage) {}
//CHECK:                                                         ^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                                                         ^^^^^ structural ray-tracing stage type 'rt.IClosestHitShader<TestHitContext>' has no runtime representation

TestClosestHit invalidStageResult();
/*CHECK:
^^^^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
^^^^^^^^^^^^^^ structural ray-tracing stage type 'TestClosestHit' has no runtime representation
*/

struct InvalidPropertyContainer
{
    property rt::ClosestHitInput<TestHitContext> invalidProperty { get; }
//CHECK:         ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:         ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' is compiler-provided and may only be used as a value parameter
}
