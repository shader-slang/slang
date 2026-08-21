//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
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
