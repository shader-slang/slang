//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#vocabulary
//META: doc_section_digest=365121cb55fb6d5257e6b445025431925b17b94447b05e87b83edaa34ae75dc3
//META: purpose=The Shader Model version family is a chain ordered by inheritance, so a feature whose floor is `sm_6_3` is accepted both at that exact floor and under any higher profile in the same chain.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that membership in a version family is inherited upward
// along the chain, not matched exactly: a later Shader Model carries
// every atom of the earlier ones. Inline ray tracing carries an
// `sm_6_3` floor, so the same kernel is compiled at exactly `sm_6_3`
// and again at `sm_6_6`, both under `-restrictive-capability-check`
// so a missing atom would be an error rather than an upgrade
// warning. Each CHECK asserts the `TraceRayInline` call survived to
// the emitted HLSL, which only happens if the profile's capability
// set satisfied the floor. The rejecting side of the same chain is
// covered by `sm-6-3-feature-on-sm-6-0-rejected-restrictive.slang`.

//TEST:SIMPLE(filecheck=FLOOR):-target hlsl -profile sm_6_3 -entry main -stage compute -restrictive-capability-check
//TEST:SIMPLE(filecheck=ABOVE):-target hlsl -profile sm_6_6 -entry main -stage compute -restrictive-capability-check

RaytracingAccelerationStructure scene;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> rq;
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    rq.TraceRayInline(scene, 0, 0xff, ray);
    outBuf[tid.x] = 1;
}

// FLOOR: TraceRayInline
// ABOVE: TraceRayInline
