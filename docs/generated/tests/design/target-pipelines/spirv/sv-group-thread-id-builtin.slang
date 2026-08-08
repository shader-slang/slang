//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=94ee0f151ec728ab5cca6e847c6705a05442b8b88f430ce9485fc71aeb34cc94
//META: purpose=SV_GroupThreadID (per-thread local id) lowers to gl_LocalInvocationID with BuiltIn LocalInvocationId, distinct from SV_GroupIndex (a flat uint scalar).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: legalizeEntryPointsForGLSL also rewrites
// SV_GroupThreadID (the three-component local id) — distinct from
// SV_GroupIndex (the flattened scalar) — into the gl_LocalInvocationID
// builtin variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 gtid : SV_GroupThreadID)
{
    outBuf[gtid.x] = gtid.x;
}

// CHECK: OpDecorate %gl_LocalInvocationID BuiltIn LocalInvocationId
