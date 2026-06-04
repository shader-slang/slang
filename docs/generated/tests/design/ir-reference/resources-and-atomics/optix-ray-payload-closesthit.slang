//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#getoptixraypayloadptr
//META: doc_section_digest=c3a35e4b5037da8ad936ccd365aac116a4da968dbaab5a0d940274845cb0ca71
//META: purpose=A `closesthit` entry point on `-target cuda` with a payload too large for inline registers surfaces the documented `getOptiXRayPayloadPtr` and `getOptiXHitAttribute` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's raytracing-payload table lists `getOptiXRayPayloadPtr` and
// `getOptiXHitAttribute` as the OptiX-side opcodes that materialize
// payload/attribute access in a `closesthit` entry point. The opcodes
// are synthesized inside `slang-ir-legalize-varying-params.cpp` during
// target legalization, AFTER the `-dump-ir` snapshot point, so they
// are not observable via IR dump. The user-visible artifact of the
// lowering is the CUDA text emit calling `optixGetPayload_*`; we check
// for that instead.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage closesthit -entry main

struct LargePayload
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
    float4 e;
    float4 f;
};

[shader("closesthit")]
void main(inout LargePayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    float3 bary = float3(
        1.0 - attribs.barycentrics.x - attribs.barycentrics.y,
        attribs.barycentrics.x,
        attribs.barycentrics.y);
    payload.a = float4(bary, RayTCurrent());
}

// CHECK-DAG: optixGetPayload_
// CHECK-DAG: optixGetAttribute_
