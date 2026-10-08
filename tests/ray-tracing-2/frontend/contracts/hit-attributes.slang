//TEST:SIMPLE: -experimental-feature -target hlsl -no-codegen
//TEST:SIMPLE: -experimental-feature -target spirv -no-codegen
//TEST:SIMPLE: -experimental-feature -target cuda -no-codegen
//TEST:SIMPLE: -experimental-feature -target metal -no-codegen
//TEST:SIMPLE: -experimental-feature -entry TriangleClosestHit -stage closesthit -target metal -no-codegen
//TEST:SIMPLE: -experimental-feature -entry TriangleAnyHit -stage anyhit -target metal -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct Payload
{
    float value;
}

struct TriangleContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct CustomAttributes
{
    float value;
    // This ordinary field remains application data even though a built-in view uses the name.
    float2 barycentricCoord;
}

struct ProceduralContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = void;
}

struct TriangleClosestHit : rt::IClosestHitShader
{
    typealias Context = TriangleContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        rt::TriangleData attributes = input.attributes;
        input.payload.value = attributes.barycentricCoord.x + float(attributes.frontFacing);
    }
}

struct TriangleAnyHit : rt::IAnyHitShader
{
    typealias Context = TriangleContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value = attributes.barycentricCoord.x + float(attributes.frontFacing);
    }
}

struct ProceduralClosestHit : rt::IClosestHitShader
{
    typealias Context = ProceduralContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        CustomAttributes attributes = input.attributes;
        input.payload.value = attributes.value + attributes.barycentricCoord.x;
    }
}

struct ProceduralAnyHit : rt::IAnyHitShader
{
    typealias Context = ProceduralContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value = attributes.value + attributes.barycentricCoord.x;
    }
}

// Reading the associated type is valid without knowing which primitive a context selects.
struct GenericAnyHit<C : rt::IHitContext> : rt::IAnyHitShader
{
    typealias Context = C;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        C.Primitive.Attributes value = attributes;
    }
}

// An ordinary equality constraint supplies the proof needed to use a specific attribute type.
struct ReadTriangleAttributes<C : rt::IHitContext> : rt::IClosestHitShader
    where C.Primitive.Attributes == rt::TriangleData
{
    typealias Context = C;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        rt::TriangleData attributes = input.attributes;
        let barycentricCoord = attributes.barycentricCoord;
    }
}
