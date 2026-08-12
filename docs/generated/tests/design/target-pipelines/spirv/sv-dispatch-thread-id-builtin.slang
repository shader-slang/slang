//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=94ee0f151ec728ab5cca6e847c6705a05442b8b88f430ce9485fc71aeb34cc94
//META: purpose=SV_DispatchThreadID lowers to gl_GlobalInvocationID with BuiltIn GlobalInvocationId decoration in SPIR-V.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeEntryPointsForGLSL is reused by the SPIR-V direct-emit
// path (Phase C step 6). It rewrites the SV_DispatchThreadID
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
