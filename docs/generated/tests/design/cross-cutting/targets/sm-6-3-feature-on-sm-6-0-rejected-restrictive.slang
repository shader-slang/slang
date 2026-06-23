//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#runtime-representation
//META: doc_section_digest=bd3a8a9e5737485c9a7ddbe47c464799f12940b6a68f5d5db5a85b41bbef6560
//META: purpose=Using `RayQuery` (an `sm_6_3` feature) in an entry point with `-profile sm_6_0` under `-restrictive-capability-check` reports the Profile's Version is below the required capability and rejects with `E41013`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SM-Version axis boundary for the join described in the doc's
// Runtime-representation section: "Computing the join (intersection)
// of two capability sets ... checking whether a function's required
// capabilities are satisfied by an entry-point's promised
// capabilities." `RayQuery` carries `sm_6_3`; `-profile sm_6_0`
// promises only up to 6.0. Under `-restrictive-capability-check` the
// implicit-upgrade warning is upgraded to error E41013.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -profile sm_6_0 -entry main -stage compute -restrictive-capability-check

RaytracingAccelerationStructure scene;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entry point 'main' uses capabilities that are not part of the specified profile 'sm_6_0'. Missing capabilities are: 'sm_6_3'
*/
// CHECK: E41013
{
    RayQuery<RAY_FLAG_NONE> rq;
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0; ray.TMax = 1;
    rq.TraceRayInline(scene, 0, 0xff, ray);
    outBuf[tid.x] = 1;
}
