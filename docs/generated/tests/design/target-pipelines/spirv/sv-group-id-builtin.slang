//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=2c1360d0390bc8ea5290e8fdf3327144bc7531f6c710e535527202f2faed4659
//META: purpose=SV_GroupID lowers to gl_WorkGroupID with BuiltIn WorkgroupId in SPIR-V.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: a third compute-stage builtin — SV_GroupID maps to
// gl_WorkGroupID under legalizeEntryPointsForGLSL, completing the
// dispatch / group-id / thread-id triad. The decoration name is
// "WorkgroupId" (note the lower-case 'group' inside the camelCase).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 gid : SV_GroupID)
{
    outBuf[0] = gid.x;
}

// CHECK: OpDecorate %gl_WorkGroupID BuiltIn WorkgroupId
