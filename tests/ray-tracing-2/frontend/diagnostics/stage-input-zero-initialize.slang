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
    // Compiler-inserted initialization must also respect the private constructor.
    rt::MissInput<Context> uninitialized;
//CHECK: declaration not accessible
//CHECK: 'MissInput.init' is not accessible from the current context.

    let local = input;
    local.payload = local.record;
}
