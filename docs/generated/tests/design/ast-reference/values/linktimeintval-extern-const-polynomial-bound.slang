//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-and-why-integer-values-are-first-class-vals
//META: doc_section_digest=8e5242c70de0586f8b4e9893dc1df5d50bc1f48c7dcc1a2393d05698891625eb
//META: purpose=A generic value reference to an extern declaration stays symbolic past semantic checking as a link-time value, and a polynomial built over it propagates that property and resolves to the linked constant at emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the link-time-value hook: `extern static const int N` is not
// an ordinary constant at check time, and the array bound `N * 2 + 1`
// is a polynomial over it, so the polynomial must carry the link-time
// property through its operand. The array size is therefore not known
// until the value is resolved; the CHECKs assert that the final emitted
// text carries a fully-resolved bound of 9 (HLSL prints the array
// extent, SPIR-V prints the array-length constant). Every shader text
// target has an array-declaration syntax that spells the resolved
// extent, so the claim is observable on all of them and each gets its
// own directive.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain

extern static const int N = 4;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int data[N * 2 + 1];
    for (int i = 0; i < N * 2 + 1; ++i) { data[i] = i; }
    outputBuffer[0] = data[N * 2];
}

// 4 * 2 + 1 == 9 once the link-time value is resolved.
//HLSL: data{{[A-Za-z0-9_]*}}[int(9)]
//SPV: OpConstant %{{[A-Za-z0-9_]+}} 9
//GLSL: int data{{[A-Za-z0-9_]*}}[9];
//METAL: array<int, int(9)> data
//WGSL: data{{[A-Za-z0-9_]*}} : array<i32, i32(9)>
