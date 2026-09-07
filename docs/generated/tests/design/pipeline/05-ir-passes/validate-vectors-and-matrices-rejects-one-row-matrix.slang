//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=667952fd5b9698b654c70c639e18194281145ed55e05baa506681d6bf7a7125d
//META: purpose=The late `validateVectorsAndMatrices` check rejects a matrix whose row count is 1 on targets whose matrix limits disallow it.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Validate row's claim that `validateVectorsAndMatrices`
// runs late against the target's vector / matrix limits. A module-scope
// `matrix<float, 1, 3>` uniform is read with a thread-varying column
// index so the matrix type survives to the late check. On GLSL and
// SPIR-V the CHECK asserts the compile is rejected with the
// matrix-dimension message; the companion test covers the D3D target
// where the same shape is allowed.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<float> buf;
uniform matrix<float, 1, 3> gM;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = gM[0][int(tid.x) % 3];
}

// GLSL: matrices with 1 column or row are not supported by the current code generation target
// SPIRV: matrices with 1 column or row are not supported by the current code generation target
