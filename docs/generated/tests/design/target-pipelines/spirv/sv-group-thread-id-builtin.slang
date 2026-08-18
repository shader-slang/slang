//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
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
