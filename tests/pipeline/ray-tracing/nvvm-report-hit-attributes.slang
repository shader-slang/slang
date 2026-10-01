// Variadic and aggregate reports share the typed NVVM attribute packing path.
//TEST:SIMPLE(filecheck=REPORT): -target ptx -emit-cuda-via-nvvm -entry main -stage intersection -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=REPORT): -target ptx -emit-cuda-via-nvvm -entry main -stage intersection -capability cuda_sm_8_0 -O3 -o -

// REPORT: result code = 0
// REPORT: _optix_report_intersection_0
// REPORT: _optix_report_intersection_1
// REPORT: _optix_report_intersection_2
// REPORT: _optix_report_intersection_3
// REPORT: _optix_report_intersection_4
// REPORT: _optix_report_intersection_5
// REPORT: _optix_report_intersection_6
// REPORT: _optix_report_intersection_7
// REPORT: _optix_report_intersection_8
// REPORT: _optix_report_intersection_8

struct Attributes
{
    float4 coordinates;
    uint4 identifiers;
}

[shader("intersection")]
void main()
{
    ReportHitOptix(1.0, 19);
    ReportHitOptix(1.1, 19, -1);
    ReportHitOptix(1.2, 19, -1, 2u);
    ReportHitOptix(1.3, 19, -1, 2u, -3);
    ReportHitOptix(1.4, 19, -1, 2u, -3, 4u);
    ReportHitOptix(1.5, 19, -1, 2u, -3, 4u, -5);
    ReportHitOptix(1.6, 19, -1, 2u, -3, 4u, -5, 6u);
    ReportHitOptix(1.7, 19, -1, 2u, -3, 4u, -5, 6u, -7);
    ReportHitOptix(1.8, 19, -1, 2u, -3, 4u, -5, 6u, -7, 8u);
    Attributes attributes = {float4(0.125, 0.25, 0.5, 1.0), uint4(11, 13, 17, 19)};
    ReportHit(2.0, 23, attributes);
}

// Empty payloads must retain traversal/invocation effects before empty values are erased.
//TEST:SIMPLE(filecheck=EMPTY): -target ptx -emit-cuda-via-nvvm -entry emptyMain -stage raygeneration -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=EMPTY): -target ptx -emit-cuda-via-nvvm -entry emptyMain -stage raygeneration -capability cuda_sm_8_0 -O3 -o -

// EMPTY: result code = 0
// EMPTY: _optix_hitobject_traverse
// EMPTY: _optix_hitobject_invoke

struct Empty {}
RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void emptyMain()
{
    Empty payload = {};
    RayDesc ray = {float3(0, 0, 1), 0, float3(0, 0, -1), 10};
    HitObject object = HitObject.TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
    output[0] = object.IsHit();
    payload = object.GetAttributes<Empty>();
    HitObject.Invoke(scene, object, payload);
    output[1] = object.IsHit();
}
