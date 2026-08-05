//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=a59d860b5f6a346b23bed14b56396e2c1ea41a3005067b975c01053cca1644c4
//META: purpose=Boundary: `NaN` also has no literal spelling, so WGSL calls a synthesized `_slang_getNan` helper while HLSL/GLSL/Metal/CUDA/C++ emit the division `0.0 / 0.0` and SPIR-V encodes the quiet-NaN bit pattern as a hex-exponent `OpConstant`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the NaN edge of the float emission axis, and contrasts with the
// infinity case: CUDA and C++ have a prelude macro for infinity but not for
// NaN, so they fall back to the division form. CHECK pins each backend's
// producer so a regression that emitted a bare `nan` token — which no
// C-family shading language accepts — would fail. The value is stored into a
// buffer so it survives dead-code elimination.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float z = float(tid.x);
    outBuf[0] = (z - z) / (z - z);
}

// HLSL: (0.0f / 0.0f)
// GLSL: (0.0 / 0.0)
// SPIRV: OpConstant %float -0x1.8p+128
// METAL: (0.0 / 0.0)
// WGSL: _slang_getNan()
// CUDA: (0.0 / 0.0)
// CPP: (0.0 / 0.0)
