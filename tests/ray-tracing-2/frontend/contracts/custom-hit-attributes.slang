//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct CustomAttributes
{
    float2 value;
}

struct ProceduralContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = TestRecord;
}

struct InvalidClosestHit : rt::IClosestHitShader<TriangleContext>
{
    void invoke(rt::ClosestHitInput<TriangleContext> input)
    {
        let attributes = input.attributes;
//CHECK:                       ^^^^^^^^^^ member not found
//CHECK:                       ^^^^^^^^^^ 'attributes' is not a member of 'rt.ClosestHitInput<TriangleContext>'.
    }
}

struct InvalidProceduralClosestHit : rt::IClosestHitShader<ProceduralContext>
{
    void invoke(rt::ClosestHitInput<ProceduralContext> input)
    {
        let triangle = input.triangle;
//CHECK:                     ^^^^^^^^ member not found
//CHECK:                     ^^^^^^^^ 'triangle' is not a member of 'rt.ClosestHitInput<ProceduralContext>'.
    }
}
