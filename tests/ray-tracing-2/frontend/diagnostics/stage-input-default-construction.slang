//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing.portable;

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
    __init();
}

[require(miss)]
void cannotCreateView()
{
    // Ordinary variable checking inserts a call to the available default constructor.
    // That call still cannot supply an invocation's payload or shader record.
    rt::MissInput<Context> fresh;
//CHECK:                   ^^^^^ construction of a structural ray-tracing type
//CHECK:                   ^^^^^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code
}
