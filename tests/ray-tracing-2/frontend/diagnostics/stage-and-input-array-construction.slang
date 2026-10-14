//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

struct Miss : rt::IMissShader
{
    typealias Context = TestContext;
    void invoke(in rt::MissInput<Context> input) {}
}

typealias Inputs = rt::MissInput<TestContext>[1];
typealias Stages = Miss[1];

void cannotConstructArrays()
{
    // These calls resolve to initializer lists. They must be checked even though there is no
    // resulting InvokeExpr or variable declaration, and the constructed values are discarded.
    Inputs();
//CHECK:  ^ construction of a structural ray-tracing type
//CHECK:  ^ structural ray-tracing type 'rt.MissInput<TestContext>[1]' cannot be constructed by user code

    Stages();
//CHECK:  ^ construction of a structural ray-tracing type
//CHECK:  ^ structural ray-tracing type 'Miss[1]' cannot be constructed by user code
}
