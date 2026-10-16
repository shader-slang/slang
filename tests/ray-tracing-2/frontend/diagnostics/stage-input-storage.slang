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

// Unwrapping an optional gives type inference an Input without calling an Input constructor.
// Global and field declarations still cannot keep it, regardless of whether the type is written.
static Optional<rt::MissInput<Context>> absent = none;
static let inferredGlobal = absent.value;
//CHECK:   ^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:   ^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

struct InvalidFields : rt::IMissShader
{
    typealias Context = ::Context;
    uint ordinaryField;
    rt::MissInput<::Context> explicitInputField;
//CHECK:                     ^^^^^^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:                     ^^^^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy
//CHECK:^^^^^^^^^ construction of a structural ray-tracing type
//CHECK:^^^^^^^^^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code

    let inferredField = absent.value;
//CHECK:^^^^^^^^^^^^^ storage of a structural ray-tracing stage input
//CHECK:^^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

    static void invoke(in rt::MissInput<::Context> input) {}
}

void uniformParameter(uniform rt::MissInput<Context> input) {}
//CHECK:                                             ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                             ^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' may only be used as an 'in' parameter or an initialized local copy

void useView(in rt::MissInput<Context> input) {}

Optional<T> makeFreshOptional<T>() { return T(); }

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

    // An arbitrary generic helper can construct T. Returning Optional<T> must not exempt
    // the helper's input type argument from the normal restriction.
    makeFreshOptional<rt::MissInput<Context>>();
/*CHECK:
    ^^^^^^^^^^^^^^^^^ ray-tracing stage input cannot be used as a generic type argument
    ^^^^^^^^^^^^^^^^^ ray-tracing stage input type 'rt.MissInput<Context>' cannot be used as a generic type argument
*/

    // Optional can wrap an existing view, but cannot make this fresh input legal.
    Optional<rt::MissInput<Context>> maybe = rt::MissInput<Context>();
//CHECK:                                                           ^ construction of a structural ray-tracing type
//CHECK:                                                           ^ structural ray-tracing type 'rt.MissInput<Context>' cannot be constructed by user code
}
