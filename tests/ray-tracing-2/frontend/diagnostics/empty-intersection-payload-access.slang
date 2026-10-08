//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry Intersection -stage intersection -target cuda -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry Intersection -stage intersection -target metal -no-codegen

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
//CHECK:                             ^^^^^^^ an empty structural ray-tracing payload has no explicit value
//CHECK:                             ^^^^^^^ empty payload type 'EmptyPayload' is represented implicitly; omit the payload argument to 'trace' and do not access 'input.payload'
    }
}
