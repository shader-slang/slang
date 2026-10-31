//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -zero-initialize -no-codegen

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
    typealias Record = uint;
}

[require(miss)]
void useView(in rt::MissInput<Context> input)
{
    // Compiler-inserted initialization does not provide the stage state needed by a fresh view.
    rt::MissInput<Context> uninitialized;
//CHECK:^^^^^^^^^ construction of a structural ray-tracing type
//CHECK:^^^^^^^^^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code

    let local = input;
    local.payload = local.record;
}
