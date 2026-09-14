//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
}

struct Attributes
{
    float2 normal;
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
    typealias Record = void;
}

// A procedural AABB supplies only a candidate bound. It cannot use `NoIntersection`, because an
// intersection shader must report the actual geometry hit inside that bound.
struct InvalidProceduralGroup : rt::IHitGroup
//CHECK:                            ^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                            ^^^^^^^^^ type argument 'InvalidProceduralGroup' does not conform to the required interface 'rt.IHitGroup'
{
    typealias Context = HitContext;
    typealias ClosestHit = rt::NoClosestHit<Context>;
    typealias AnyHit = rt::NoAnyHit<Context>;
    typealias Intersection = rt::NoIntersection<Context>;
//CHECK:                         ^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                         ^^^^^^^^^^^^^^ type argument 'rt.BoundingBoxPrimitive<Attributes>' does not conform to the required interface 'rt.IBuiltinIntersectionPrimitive'
}
