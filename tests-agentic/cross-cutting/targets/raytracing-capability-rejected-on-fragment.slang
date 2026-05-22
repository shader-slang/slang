//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#capability-system
//META: doc_section_digest=a0d40ede85e7cb31a316466bcfe695e496506c5ca59e269adafbe2b4671c8e81
//META: purpose=A capability name expands to a disjunction of conjunctions (the doc's `raytracing` example). Using a raytracing-only intrinsic from a compute entry point on HLSL is rejected with a stage / capability mismatch diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Vocabulary section uses the `raytracing` alias as a
// motivating example:
//   raytracing = GL_EXT_ray_tracing | _sm_6_3 | cuda
// `TraceRay` requires a raytracing stage; calling it from a
// `compute` entry point on HLSL violates the stage keyhole — the
// `raygen`/`closesthit`/etc. atoms populate the same `stage`
// keyhole as `compute` with different values. The compiler raises a
// stage-mismatch diagnostic at the entry-point signature.

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
