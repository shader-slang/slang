//TEST:SIMPLE(filecheck=CUDAIS): -experimental-feature -entry Intersection -stage intersection -target cuda
//TEST:SIMPLE(filecheck=PTXIS): -experimental-feature -entry Intersection -stage intersection -target ptx -Xnvrtc -I"./external/optix-dev/include/"
//TEST:SIMPLE(filecheck=CUDACH): -experimental-feature -entry ClosestHit -stage closesthit -target cuda
//TEST:SIMPLE(filecheck=PTXCH): -experimental-feature -entry ClosestHit -stage closesthit -target ptx -Xnvrtc -I"./external/optix-dev/include/"

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

enum AttributeClass : uint16_t
{
    Surface = 3,
}

struct NestedAttributes
{
    half weight;
    AttributeClass attributeClass;
    uint64_t key;
}

struct CustomAttributes
{
    float2 uv;
    NestedAttributes nested;
    float samples[2];
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::BoundingBoxPrimitive<CustomAttributes>;
    typealias Record = void;
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;

    void invoke(rt::IntersectionInput<HitContext> input)
    {
        CustomAttributes attributes;
        attributes.uv = float2(0.25, 0.75);
        attributes.nested.weight = half(1.5);
        attributes.nested.attributeClass = AttributeClass.Surface;
        attributes.nested.key = 0x1122334455667788ull;
        attributes.samples[0] = 2.0;
        attributes.samples[1] = 4.0;

        // Exercise both source overloads. They must use the same eight-word aggregate transport;
        // only the second call supplies a nonzero hit kind.
        if (!input.reportHit(1.0, attributes))
            input.reportHit(2.0, 7, attributes);
    }
}

// The aggregate occupies eight OptiX registers: float2 (2), half (1), enum (1), uint64_t (2),
// and float[2] (2). Both source overloads must reach the native OptiX operation.
// CUDAIS-COUNT-2: optixReportIntersection(
// CUDAIS-NOT: rt_IntersectionInput_reportHit
// PTXIS-COUNT-2: _optix_report_intersection_8

RWStructuredBuffer<uint> output;

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(rt::ClosestHitInput<HitContext> input)
    {
        let attributes = input.attributes;
        output[0] = asuint(attributes.uv.x) + asuint(attributes.uv.y) +
                    uint(attributes.nested.weight) + uint(attributes.nested.attributeClass) +
                    uint(attributes.nested.key) + uint(attributes.nested.key >> 32) +
                    asuint(attributes.samples[0]) + asuint(attributes.samples[1]);
    }
}

// The consumer reconstructs the same eight-word sequence that `reportHit` emits.
// CUDACH-DAG: optixGetAttribute_0()
// CUDACH-DAG: optixGetAttribute_1()
// CUDACH-DAG: optixGetAttribute_2()
// CUDACH-DAG: optixGetAttribute_3()
// CUDACH-DAG: optixGetAttribute_4()
// CUDACH-DAG: optixGetAttribute_5()
// CUDACH-DAG: optixGetAttribute_6()
// CUDACH-DAG: optixGetAttribute_7()
// PTXCH-DAG: _optix_get_attribute_0
// PTXCH-DAG: _optix_get_attribute_7
