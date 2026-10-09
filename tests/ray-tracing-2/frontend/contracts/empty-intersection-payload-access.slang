//TEST:SIMPLE: -experimental-feature -entry Intersection -stage intersection -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Intersection -stage intersection -target metal -no-codegen

// Empty payloads are ordinary source values; target legalization owns their physical storage.
import slang.raytracing;

struct EmptyPayload
{}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = EmptyPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = void;
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        EmptyPayload payload = input.payload;
        input.payload = payload;
    }
}
