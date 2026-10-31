//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing.portable;

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

typealias Inputs = rt::MissInput<TestContext>[1];

void cannotConstructArrays()
{
    // This call resolves to an initializer list. It must be checked even though there is no
    // resulting InvokeExpr or variable declaration, and the constructed value is discarded.
    Inputs();
//CHECK:  ^ construction of a structural ray-tracing type
//CHECK:  ^ structural ray-tracing type 'rt.MissInput<TestContext>[1]' cannot be constructed by user code
}
