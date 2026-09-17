//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen

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

// Triangle intersection is supplied by the target. Adding a source intersection stage would
// compete with that built-in primitive contract and must therefore fail at conformance checking.
struct InvalidTriangleIntersection : rt::IIntersectionShader
//CHECK:                                 ^^^^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:                                 ^^^^^^^^^^^^^^^^^^^ type argument 'InvalidTriangleIntersection' does not conform to the required interface 'rt.IIntersectionShader'
{
    typealias Context = HitContext;

    void invoke(rt::IntersectionInput<HitContext> input) {}
}
