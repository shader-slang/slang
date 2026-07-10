//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=2c1360d0390bc8ea5290e8fdf3327144bc7531f6c710e535527202f2faed4659
//META: purpose=SV_GroupIndex lowers to gl_LocalInvocationIndex with BuiltIn LocalInvocationIndex decoration in SPIR-V.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeEntryPointsForGLSL also rewrites SV_GroupIndex to the
// gl_LocalInvocationIndex input variable. This is the second
// canonical compute-stage system-value lowering.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint gi : SV_GroupIndex, uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gi;
}

// CHECK: OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
