// A generic extension can implement a structural ray-tracing stage interface: the clause names
// its own generic parameter as the target, so the user-defined-struct rule is checked against
// each concrete type the extension applies to, not against the parameter.
// The whole-module compile accepts an unused generic stage extension, and direct entry selection
// finds the specialized static invoke supplied by the extension.
//TEST:SIMPLE: -experimental-feature -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Hit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Hit.invoke -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry HitWithOrdinaryData -stage closesthit -no-codegen

import slang.raytracing.portable;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

interface IMarker {}

__generic<T : IMarker>
extension T : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(in rt::ClosestHitInput<Context> input) {}
}

struct Hit : IMarker {}

// Ordinary data fields stay ordinary struct data: stage dispatch calls the static invoke
// without constructing a value or running a constructor.
struct HitWithOrdinaryData : IMarker
{
    uint state;
}
