//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=5fecd0eb9a12a63e0294d7fc25d4ccc9d8e7cc1d7f20cc7a08f9fadaa7ec7ae7
//META: purpose=SV_DispatchThreadID lowers to gl_GlobalInvocationID with BuiltIn GlobalInvocationId decoration in SPIR-V.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeEntryPointsForGLSL is reused by the SPIR-V direct-emit
// path (Phase C step 3). It rewrites the SV_DispatchThreadID
// system-value parameter as a gl_GlobalInvocationID input variable
// decorated BuiltIn GlobalInvocationId.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}

// CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
