//TEST:SIMPLE: -experimental-feature -entry Intersection -stage intersection -target cuda -no-codegen

// Adapter synthesis is outside this frontend slice. Successful checking for the CUDA target proves
// that both structural `reportHit` overloads resolve for an OptiX intersection stage. Target tests
// will verify their later lowering to `optixReportIntersection` once adapters are enabled.

import slang.raytracing;

struct Payload {}
struct Record {}

struct Attributes
{
    float2 uv;
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
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = ::Record;
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(in rt::IntersectionInput<Context> input)
    {
        Attributes attributes = {};
        input.reportHit(1.0, attributes);
        input.reportHit(2.0, 7, attributes);
    }
}
