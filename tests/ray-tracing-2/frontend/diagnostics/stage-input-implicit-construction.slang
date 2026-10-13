//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Context : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

extension rt::MissInput<Context>
{
    __implicit_conversion(100)
    __init(int ignored) {}
}

[require(miss)]
void useView(in rt::MissInput<Context> input) {}

[require(miss)]
void cannotCreateView()
{
    // An implicit constructor cannot manufacture a view for a helper's input parameter.
    useView(1);
//CHECK:    ^ construction of a structural ray-tracing type
//CHECK:    ^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code
}
