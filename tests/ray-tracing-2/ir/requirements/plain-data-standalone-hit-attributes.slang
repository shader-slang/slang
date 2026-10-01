//DIAGNOSTIC_TEST:SIMPLE(filecheck=CH): -experimental-feature -entry BadClosestHit -stage closesthit -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(filecheck=AH): -experimental-feature -entry BadAnyHit -stage anyhit -target hlsl -profile sm_6_5
//DIAGNOSTIC_TEST:SIMPLE(filecheck=IS): -experimental-feature -entry BadIntersection -stage intersection -target hlsl -profile sm_6_5

import slang.raytracing;

struct PayloadData
{
    uint value;
}

struct InvalidAttributes
// CH: structural ray-tracing ABI data is not portable plain data
// CH: ^^^^^^^^^^^^^^^^^ type 'InvalidAttributes' is used as structural ray-tracing intersection-attribute data but does not have a fixed-size, copyable plain-data representation
// AH: structural ray-tracing ABI data is not portable plain data
// AH: ^^^^^^^^^^^^^^^^^ type 'InvalidAttributes' is used as structural ray-tracing intersection-attribute data but does not have a fixed-size, copyable plain-data representation
// IS: structural ray-tracing ABI data is not portable plain data
// IS: ^^^^^^^^^^^^^^^^^ type 'InvalidAttributes' is used as structural ray-tracing intersection-attribute data but does not have a fixed-size, copyable plain-data representation
{
    Texture2D texture;
}

struct SceneTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = SceneTraceContext;
    typealias Payload = PayloadData;
    typealias Primitive = rt::BoundingBoxPrimitive<InvalidAttributes>;
    typealias Record = void;
}

struct BadClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<Context> input) {}
}

struct BadAnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<Context> input) {}
}

struct BadIntersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<Context> input) {}
}
