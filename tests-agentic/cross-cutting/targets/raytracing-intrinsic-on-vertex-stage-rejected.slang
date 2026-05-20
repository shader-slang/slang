//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#capability-system
//META: doc_section_digest=a0d40ede85e7cb31a316466bcfe695e496506c5ca59e269adafbe2b4671c8e81
//META: purpose=The `raytracing` capability requires a raygen/closesthit/... stage atom; calling `TraceRay` from a `vertex` entry point populates the `stage` keyhole with `vertex`, conflicting with the raytracing-stage atoms and producing a stage-mismatch diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary companion of `raytracing-capability-rejected-on-fragment.slang`
// at a different stage. The doc's Vocabulary section uses the
// `raytracing` alias as the canonical example of a disjunction-of-
// conjunctions; its stage atom conflicts with `vertex`. Slang must
// reject the call site at the entry-point signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -profile vs_6_5

struct Payload { float v; };

uniform RaytracingAccelerationStructure scene;

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
/*CHECK:
       ^^^^ entrypoint 'main' uses features that are not available in 'vertex' stage for 'hlsl' compilation target.
*/
{
    Payload p;
    p.v = 0;
    RayDesc r;
    r.Origin = float3(0, 0, 0);
    r.Direction = float3(0, 0, 1);
    r.TMin = 0; r.TMax = 1;
    TraceRay(scene, 0, 0xff, 0, 1, 0, r, p);
    return float4(p.v, 0, 0, 1);
}
