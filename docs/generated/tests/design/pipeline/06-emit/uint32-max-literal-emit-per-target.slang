//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=a59d860b5f6a346b23bed14b56396e2c1ea41a3005067b975c01053cca1644c4
//META: purpose=Boundary: the `uint` maximum `4294967295` survives literal emission on every backend — the C-family targets append the `U` suffix that keeps the literal unsigned, WGSL wraps it in `u32(...)`, and SPIR-V encodes it as an `OpConstant %uint`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper edge of the unsigned 32-bit literal axis at the emit
// boundary. Without the `U` suffix a C-family downstream compiler would read
// `4294967295` as a signed value that does not fit in `int`, so the suffix is
// the load-bearing part of the emission. CHECK asserts each backend's
// spelling; the value is stored into a buffer so it cannot be eliminated.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = 4294967295u;
}

// HLSL: = 4294967295U;
// GLSL: = 4294967295U;
// SPIRV: OpConstant %uint 4294967295
// METAL: = 4294967295U;
// WGSL: u32(4294967295)
// CUDA: = 4294967295U;
// CPP: = 4294967295U;
