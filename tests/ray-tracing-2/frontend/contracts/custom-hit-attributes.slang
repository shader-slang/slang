//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -experimental-feature -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
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
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = TestRecord;
}

struct InvalidClosestHit : rt::IClosestHitShader
{
    typealias Context = TriangleContext;
    void invoke(rt::ClosestHitInput<TriangleContext> input)
    {
        let attributes = input.attributes;
        let value = attributes.value;
//CHECK: 'value' is not a member of 'rt.TriangleData'.
    }
}

struct InvalidProceduralAnyHit : rt::IAnyHitShader
{
    typealias Context = ProceduralContext;
    void invoke(rt::AnyHitInput<ProceduralContext> input)
    {
        let attributes = input.attributes;
        let frontFacing = attributes.frontFacing;
//CHECK: 'frontFacing' is not a member of 'CustomAttributes'.
    }
}

// A generic stage must prove the attribute type before assuming a triangle view. A concrete
// specialization cannot make an otherwise invalid generic body well-typed.
struct InvalidGenericClosestHit<C : rt::IHitContext> : rt::IClosestHitShader
{
    typealias Context = C;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        rt::TriangleData attributes = input.attributes;
//CHECK: expected an expression of type 'rt.TriangleData', got 'C.Primitive.Attributes'
    }
}

struct InvalidGenericAnyHit<C : rt::IHitContext> : rt::IAnyHitShader
{
    typealias Context = C;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        let barycentricCoord = attributes.barycentricCoord;
//CHECK: 'barycentricCoord' is not a member of 'C.Primitive.Attributes'.
    }
}
