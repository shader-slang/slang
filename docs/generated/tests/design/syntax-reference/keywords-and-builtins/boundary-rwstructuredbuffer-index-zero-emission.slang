//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=12582dfa01eb81163b717e59e208875fece8e3cb827d3738a483bbc260d0c7d8
//META: purpose=Boundary: the HLSL-flavored buffer vocabulary contributed by `hlsl.meta.slang` reaches every text-emit back-end, and a write at the lower-edge index 0 survives into each target's own indexing form.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a buffer type named by hlsl.meta.slang is not an
// HLSL-only spelling: the same `outBuf[0] = 7;` write is lowered to
// HLSL, GLSL, Metal, SPIR-V, and WGSL. Each CHECK pins that target's
// own rendering of the lower-edge index (HLSL `[int(0)]`, GLSL
// `[uint(0)]`, Metal's `+int(0)` pointer offset, the SPIR-V `%int_0`
// constant, WGSL `[i32(0)]`), so the index-zero write is observed on
// every back-end rather than assumed from one.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 7;
}

//HLSL: outBuf_{{[0-9]+}}[int(0)] = int(7)
//GLSL: _data[uint(0)] = 7
//METAL: outBuf_{{[0-9]+}}+int(0)
//SPV: %int_0 = OpConstant %int 0
//WGSL: outBuf_0[i32(0)] = i32(7)
