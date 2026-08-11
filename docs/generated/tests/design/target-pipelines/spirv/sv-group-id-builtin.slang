//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=94ee0f151ec728ab5cca6e847c6705a05442b8b88f430ce9485fc71aeb34cc94
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
