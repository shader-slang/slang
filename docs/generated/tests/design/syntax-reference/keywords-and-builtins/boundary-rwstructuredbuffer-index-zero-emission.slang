//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T00:00:00+00:00
//META: source_commit=1ee81dae21143fb69ccb9999f0fd013b4db2d922
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Boundary: `RWStructuredBuffer<T>` from `hlsl.meta.slang` survives into HLSL emit with an `int(0)` indexing expression at the documented lower-edge index.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Emission fan-out for the RWStructuredBuffer lower-edge (index 0) write.
// The sibling `boundary-rwstructuredbuffer-index-zero-hlsl.slang` pins
// the HLSL form; here the same `outBuf[0] = 7;` write is lowered to GLSL,
// Metal, SPIR-V, and WGSL. Each CHECK pins the target's own lowering of
// the zero index (GLSL `[uint(0)]`, Metal `+int(0)` pointer offset,
// SPIR-V the `%int_0` constant, WGSL `[i32(0)]`) so the lower-edge index
// is observed on every shader back-end, not just HLSL.

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

//GLSL: _data[uint(0)] = 7
//METAL: outBuf_{{[0-9]+}}+int(0)
//SPV: %int_0 = OpConstant %int 0
//WGSL: outBuf_0[i32(0)] = i32(7)
