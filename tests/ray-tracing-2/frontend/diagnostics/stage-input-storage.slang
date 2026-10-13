//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

import slang.raytracing;

struct Payload {}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Context : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Record = ::Record;
}

// A stage view cannot be supplied by host parameter binding or by global storage.
rt::MissInput<Context> globalInput;
//CHECK:               ^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:               ^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

uniform rt::MissInput<Context> uniformInput;
//CHECK:                       ^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                       ^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

static rt::MissInput<Context> staticInput;
//CHECK:                      ^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                      ^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

void uniformParameter(uniform rt::MissInput<Context> input) {}
//CHECK:                                             ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                             ^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

void useView(in rt::MissInput<Context> input) {}

void cannotCreateView(in rt::MissInput<Context> input)
{
    rt::MissInput<Context> uninitialized;
//CHECK:                   ^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                   ^^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

    rt::MissInput<Context> empty = {};
//CHECK:                           ^ construction of a structural ray-tracing type
//CHECK:                           ^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code

    rt::MissInput<Context>();
//CHECK:                  ^ construction of a structural ray-tracing type
//CHECK:                  ^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code

    useView({});
//CHECK:    ^ construction of a structural ray-tracing type
//CHECK:    ^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code
}
