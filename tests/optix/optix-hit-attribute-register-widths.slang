//TEST:SIMPLE(filecheck=FETCH): -target cuda -entry closestHit -stage closesthit
//TEST:SIMPLE(filecheck=REPORT): -target cuda -entry intersection -stage intersection
//TEST:SIMPLE(filecheck=FETCH-PTX): -target ptx -entry closestHit -stage closesthit
//TEST:SIMPLE(filecheck=REPORT-PTX): -target ptx -entry intersection -stage intersection

enum AttributeKind : uint16_t
{
    Primary = 1,
    Secondary = 2,
}

struct WideAttributes
{
    double distance;        // attributes 0-1, low word first
    uint64_t key;           // attributes 2-3, low word first
    half weight;            // attribute 4, bit pattern in the low 16 bits
    AttributeKind kind;     // attribute 5, underlying uint16 value
    float2 textureCoord;    // attributes 6-7
}

struct Payload
{
    float value;
}

// A native OptiX attribute is always one uint32 word. The compiler reconstructs wider and narrower
// source scalars from that word stream instead of asking the CUDA emitter to return those source
// types directly from `optixGetAttribute_N()`.
// FETCH-LABEL: __closesthit__closestHit()
// FETCH-DAG: optixGetAttribute_0()
// FETCH-DAG: optixGetAttribute_1()
// FETCH-DAG: optixGetAttribute_2()
// FETCH-DAG: optixGetAttribute_3()
// FETCH-DAG: optixGetAttribute_4()
// FETCH-DAG: optixGetAttribute_5()
// FETCH-DAG: optixGetAttribute_6()
// FETCH-DAG: optixGetAttribute_7()
// FETCH-PTX-DAG: _optix_get_attribute_0
// FETCH-PTX-DAG: _optix_get_attribute_1
// FETCH-PTX-DAG: _optix_get_attribute_2
// FETCH-PTX-DAG: _optix_get_attribute_3
// FETCH-PTX-DAG: _optix_get_attribute_4
// FETCH-PTX-DAG: _optix_get_attribute_5
// FETCH-PTX-DAG: _optix_get_attribute_6
// FETCH-PTX-DAG: _optix_get_attribute_7
[shader("closesthit")]
void closestHit(inout Payload payload, in WideAttributes attributes)
{
    payload.value = float(attributes.distance) + float(attributes.key) + float(attributes.weight) +
                    float(attributes.kind) + attributes.textureCoord.x +
                    attributes.textureCoord.y;
}

// `ReportHitOptix` is the legacy CUDA producer API: its uint arguments already are the physical
// word stream. The ordering here mirrors `WideAttributes`, providing an independent producer for
// the closest-hit reconstruction above. Structural aggregate reportHit lowering will reuse the
// same compiler-owned plan in its dependent change.
// REPORT: optixReportIntersection(1.0f, 7U,
// REPORT-SAME: 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U)
// REPORT-PTX: _optix_report_intersection_8
[shader("intersection")]
void intersection()
{
    ReportHitOptix(1.0, 7, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U);
}
