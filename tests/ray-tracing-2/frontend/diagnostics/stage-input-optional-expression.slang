//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -ignore-capabilities -no-codegen

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
    typealias Record = uint;
}

typealias Input = rt::MissInput<Context>;
interface IMarker {}
extension Input : IMarker {}

void useInput(in Input input) {}
IMarker box(in Input input) { return input; }

void cannotRecoverInput(IMarker other, in Input input)
{
    // Each cast produces Optional<Input>, even when the optional has no named local.
    useInput((other as Input).value);
//CHECK:            ^^ storage of a structural ray-tracing stage input
//CHECK:            ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy

    Input local = (other as Input).value;
//CHECK:                 ^^ storage of a structural ray-tracing stage input
//CHECK:                 ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy

    bool present = (other as Input).hasValue;
//CHECK:                  ^^ storage of a structural ray-tracing stage input
//CHECK:                  ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy

    (((other as Input)));
//CHECK:     ^^ storage of a structural ray-tracing stage input
//CHECK:     ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy

    // Knowing that the cast succeeds still does not permit Optional<Input>.
    useInput((input as Input).value);
//CHECK:            ^^ storage of a structural ray-tracing stage input
//CHECK:            ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy

    // Boxing an existing view must not allow recovering it through an optional.
    useInput((box(input) as Input).value);
//CHECK:                 ^^ storage of a structural ray-tracing stage input
//CHECK:                 ^^ ray-tracing stage input type 'Optional<rt.MissInput<Context>>' may only be used as an 'in' parameter or an initialized local copy
}

void cannotWrapGenericContext<C : rt::IPayloadContext>(in rt::MissInput<C> input)
{
    (input as rt::MissInput<C>);
//CHECK:   ^^ storage of a structural ray-tracing stage input
//CHECK:   ^^ ray-tracing stage input type 'Optional<rt.MissInput<C>>' may only be used as an 'in' parameter or an initialized local copy
}
