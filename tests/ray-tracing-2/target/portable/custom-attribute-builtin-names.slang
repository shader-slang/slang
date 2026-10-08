//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry AnyHit -stage anyhit -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

import slang.raytracing;

struct Payload
{
    float value;
}

// These are application fields, not TriangleData/CurveData properties. Lowering must use the
// nominal attribute type rather than recognizing a field by its spelling.
struct Attributes
{
    float2 barycentricCoord;
    uint frontFacing;
    float parameter;
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

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value += attributes.barycentricCoord.x + attributes.frontFacing
            + attributes.parameter;
    }
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;

    void invoke(rt::AnyHitInput<Context> input)
    {
        let attributes = input.attributes;
        input.payload.value = attributes.barycentricCoord.y + attributes.frontFacing
            + attributes.parameter;
    }
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(rt::IntersectionInput<Context> input)
    {
        Attributes attributes = {float2(3.0, 5.0), 7, 11.0};
        input.reportHit(1.0, attributes);
    }
}

struct HitGroup : rt::IHitGroup
{
    typealias Context = HitContext;
    typealias ClosestHit = ::ClosestHit;
    typealias AnyHit = ::AnyHit;
    typealias Intersection = ::Intersection;
}

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::HitGroupList<HitGroup>;
    typealias MissShaders = rt::NoMissShaders;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;
RWStructuredBuffer<float> output;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    output[0] = payload.value;
}

// HLSL: struct Attributes_0
// HLSL: float2 barycentricCoord_0;
// HLSL: uint frontFacing_0;
// HLSL: float parameter_0;
// HLSL: [shader("closesthit")]void ClosestHit(
// HLSL-SAME: Attributes_0
// HLSL-SAME: SV_IntersectionAttributes
// HLSL-NOT: HitKind()
// SPIRV: OpVariable %_ptr_HitAttributeKHR_Attributes HitAttributeKHR
// METAL: intersection_function_table<metal::raytracing::instancing>
// METAL-NOT: triangle_data
// METAL-NOT: curve_data
// METAL: {{\[\[}}intersection(bounding_box, metal::raytracing::instancing)]]
