// Queries that merely probe a generic stage extension stay silent: abstract shapes (a generic
// parameter, an associated type, an interface's `This`) are not concrete implementations, so
// their applications defer instead of diagnosing, and speculative overload probes on valid
// concrete types produce no diagnostics at all.
//TEST:SIMPLE: -experimental-feature -no-codegen

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

struct GoodHit : IMarker
{
    uint state;
}

struct Box<T : IMarker> : IMarker
{
    T inner;
}

// An interface refining the marker contributes associated-type and `This` shapes that must
// defer like a generic parameter does.
interface IRefines : IMarker
{
    associatedtype Assoc : IMarker;
}

void useStage<T : rt::IClosestHitShader>() {}

// Failed overload candidates probe the marker constraint on types that never conform to it;
// computing facets for a valid stage type along the way must not diagnose anything.
int choose(float value)
{
    return 1;
}

int choose<T : IMarker>(T value)
{
    return 2;
}

void assocProbe<I : IRefines>()
{
    useStage<I.Assoc>();
}

void thisProbe<I : IRefines>()
{
    // The refining interface itself is not a concrete stage implementation.
    useStage<I>();
}

void genericProbe<T : IMarker>()
{
    // A generic parameter inside another generic defers to its concrete instantiations.
    useStage<T>();
}

[numthreads(1, 1, 1)]
void main()
{
    useStage<GoodHit>();
    useStage<Box<GoodHit>>();
    choose(1.0f);
}
