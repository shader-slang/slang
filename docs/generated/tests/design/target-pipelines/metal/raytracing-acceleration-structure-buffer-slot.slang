//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=7b7f20ff0637d9f1a655a69590f780c00b292d49ad138092b4ca9bd4ebed1ccb
//META: purpose=A `RaytracingAccelerationStructure` emits as `metal::raytracing::acceleration_structure<metal::raytracing::instancing>` and takes a `[[buffer(N)]]` slot.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the acceleration-structure row of the resource table, which is
// the one resource whose MSL spelling is a fully qualified template rather
// than a bare type name, and which shares the buffer slot space with plain
// data buffers rather than having a space of its own. Tracing an inline ray
// against it keeps the parameter live to emit.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RaytracingAccelerationStructure accel;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.0;
    ray.TMax = 100.0;
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(accel, RAY_FLAG_NONE, 0xff, ray);
    q.Proceed();
    outBuf[0] = q.CommittedRayT();
}

// CHECK: metal::raytracing::acceleration_structure<metal::raytracing::instancing> accel_{{[0-9]+}} {{\[\[}}buffer({{[0-9]+}}){{\]\]}}
