//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#capability-system
//META: doc_section_digest=ae73a8962b418df71d4a8a3d2d9b722c9933a29ec7eefb63e8b0b9e081f0fe14
//META: purpose=A capability name such as `raytracing` expands to a disjunction of conjunctions whose stage atoms exclude `compute`, so calling `TraceRay` from a compute entry point on HLSL is rejected with `E36107`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a capability name is a Boolean expression over atoms
// and that none of its alternatives admits the active stage: every
// disjunct behind `raytracing` populates the `stage` keyhole with a
// ray-tracing stage, which conflicts with the `compute` atom the
// entry point already contributes. `TraceRay` is called from a
// compute entry point and the CHECK pins E36107 on the entry-point
// signature, which is where the join of required-against-promised
// capabilities is reported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -profile cs_6_5

struct Payload { float v; };

uniform RaytracingAccelerationStructure scene;
uniform RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'hlsl' compilation target.
*/
//CHECK: E36107
{
    Payload p;
    p.v = 0;
    RayDesc r;
    r.Origin = float3(0, 0, 0);
    r.Direction = float3(0, 0, 1);
    r.TMin = 0; r.TMax = 1;
    TraceRay(scene, 0, 0xff, 0, 1, 0, r, p);
    outBuf[tid.x] = p.v;
}
