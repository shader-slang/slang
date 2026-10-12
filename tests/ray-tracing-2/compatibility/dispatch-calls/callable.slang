//TEST:SIMPLE: -experimental-feature -entry StructuralClosestHit -stage closesthit -no-codegen

// A structural stage can call a helper that uses the native CallShader function.
// The helper is an ordinary function; it does not declare a native callable shader.
import slang.raytracing;

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

struct StructuralClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        dispatchCallable(input.payload);
    }
}

void dispatchCallable(inout Payload payload)
{
    CallShader(0, payload);
}
