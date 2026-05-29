//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#downstream-spirv-link-spirv-val-spirv-opt-chain
//META: doc_section_digest=d577a3bf24503ab3e84d5636353894e45b6ae43c402e71aef987cd10320bdd2a
//META: purpose=spirv-val runs when -validate-spirv is passed (or SLANG_RUN_SPIRV_VALIDATION is set) — on a valid module the emit is unchanged and still carries the canonical OpCapability Shader / OpMemoryModel Logical GLSL450 prelude.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: -validate-spirv enables spirv-val (Phase D, step 18).
// shouldRunSPIRVValidation is true when the CLI flag is set or
// SLANG_RUN_SPIRV_VALIDATION is non-empty (and -skip-spirv-validation
// is off). On a valid module the validator passes silently and the
// emitted assembly is unchanged. This test exercises the validation
// code path; the FileCheck patterns verify the canonical prelude
// still emits.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -validate-spirv

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x);
}

// CHECK: OpCapability Shader
// CHECK: OpMemoryModel Logical GLSL450
// CHECK: OpEntryPoint GLCompute %main
