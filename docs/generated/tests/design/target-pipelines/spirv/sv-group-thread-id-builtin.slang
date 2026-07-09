//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=2c1360d0390bc8ea5290e8fdf3327144bc7531f6c710e535527202f2faed4659
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
