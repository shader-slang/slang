//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#resource-type-spellings
//META: doc_section_digest=7b7f20ff0637d9f1a655a69590f780c00b292d49ad138092b4ca9bd4ebed1ccb
//META: purpose=A `RayQuery` becomes a `raytracing::intersection_query<raytracing::triangle_data, raytracing::instancing>` local and takes no binding attribute at all.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one row of the resource table whose Attribute column is empty:
// a ray query is a thread-local object, not a bound resource, so it is
// declared inside the kernel body with Metal's thread address space and
// never appears in the parameter list. The CHECK pins the two template
// arguments the emitter always supplies.

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

// CHECK: thread raytracing::intersection_query<raytracing::triangle_data, raytracing::instancing> q_
