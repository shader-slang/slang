//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda
//META: doc_section_digest=d141689276dbc4a39e279f1adba85a6899409f5555430cdbc264fa99d8cea1a1
//META: purpose=A shader-terminating IgnoreHit buried in an ordinary (non-ForceInline) callee is inlined into the anyhit entry point so the payload write-back is emitted immediately before optixIgnoreIntersection.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the inlining pre-pass that
// `legalizeEntryPointVaryingParamsForCUDA` runs before its ordinary
// parameter legalization. `maybeIgnore` is a plain callee — no
// `[ForceInline]` — that mutates the payload and then calls `IgnoreHit()`.
// Payload write-backs are inserted only into the entry point's own
// blocks, so without the inlining step the mutation would be lost. The
// CHECKs assert that the mutation, the four `optixSetPayload_*` writes,
// and `optixIgnoreIntersection()` all appear inside the emitted
// `__anyhit__` kernel, in that order — the callee body was flattened in
// and the write-back precedes the terminate call.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry anyHitMain -stage anyhit

struct Payload
{
    float4 color;
};

void maybeIgnore(inout Payload p, float t)
{
    if (t < 0.5f)
    {
        p.color.x = 1.0f;
        IgnoreHit();
    }
}

[shader("anyhit")]
void anyHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    maybeIgnore(payload, attribs.barycentrics.x);
    payload.color.y = 2.0f;
}

// CHECK: extern "C" __global__ void __anyhit__anyHitMain
// The inlined callee's payload mutation ...
// CHECK: = 1.0f
// ... is written back before the ray is terminated.
// CHECK: optixSetPayload_0(
// CHECK: optixIgnoreIntersection()
