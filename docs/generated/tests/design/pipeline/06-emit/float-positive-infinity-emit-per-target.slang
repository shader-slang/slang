//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=4b10e63cd2245887946d5a04f643460694312535bff48052c288d542dac985ca
//META: purpose=Boundary: `+inf` has no literal spelling in most shading languages, so each backend emits its own producer — CUDA and C++ use the prelude's `SLANG_INFINITY`, WGSL calls a synthesized `_slang_getInfinity` helper, and HLSL/GLSL/Metal emit the division `1.0 / 0.0`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the infinity edge of the float emission axis. There is no portable
// `inf` token, so the backends diverge and the test's value is exactly that
// divergence. CHECK asserts the specific producer each backend chose; a
// backend that regressed to printing `inf` or `1.#INF` would fail its own
// prefix. SPIR-V is included because it can encode the bit pattern directly
// as a hex-exponent `OpConstant`.

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
    outBuf[0] = 1.0f / (z - z);
}

// HLSL: (1.0f / 0.0f)
// GLSL: (1.0 / 0.0)
// SPIRV: OpConstant %float 0x1p+128
// METAL: (1.0 / 0.0)
// WGSL: _slang_getInfinity(true)
// CUDA: SLANG_INFINITY
// CPP: SLANG_INFINITY
