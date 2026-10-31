//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

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

struct TestClosestHit : rt::IClosestHitShader
{
    typealias Context = TestHitContext;
    static void invoke(rt::ClosestHitInput<TestHitContext> input) {}
}

struct Box<T>
{
    T value;
}

[require(closesthit)]
void invalidStorage(
        TestClosestHit stage,
        rt::ClosestHitInput<TestHitContext> input)
{
    rt::ClosestHitInput<TestHitContext> localInput;
//CHECK:                                ^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                ^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' may only be used as an 'in' parameter or an initialized local copy

    Box<rt::ClosestHitInput<TestHitContext>> wrappedInput;
//CHECK:    ^^^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:    ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' cannot be used as a generic type argument

    Box<Box<rt::ClosestHitInput<TestHitContext>>> nestedWrappedInput;
//CHECK:        ^^^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
//CHECK:        ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' cannot be used as a generic type argument

    rt::ClosestHitInput<TestHitContext>();
//CHECK:                               ^ construction of a structural ray-tracing type
//CHECK:                               ^ structural ray-tracing type 'rt.ClosestHitInput<TestHitContext>' cannot be constructed by user code
}

rt::ClosestHitInput<TestHitContext> invalidResult();
/*CHECK:
    ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
    ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' may only be used as an 'in' parameter or an initialized local copy
*/

[require(closesthit)]
void invalidInterfaceValue(rt::IClosestHitShader stage) {}
//CHECK:                                         ^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                                         ^^^^^ structural ray-tracing stage type 'rt.IClosestHitShader' has no runtime representation

typealias InvalidOptionalStageInterface = Optional<rt::IMissShader>;
//CHECK:                                               ^^^^^^^^^^^ runtime use of a structural ray-tracing stage type
//CHECK:                                               ^^^^^^^^^^^ structural ray-tracing stage type 'rt.IMissShader' has no runtime representation

struct InvalidPropertyContainer
{
    property rt::ClosestHitInput<TestHitContext> invalidProperty { get; }
//CHECK:         ^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:         ^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<TestHitContext>' may only be used as an 'in' parameter or an initialized local copy
}
