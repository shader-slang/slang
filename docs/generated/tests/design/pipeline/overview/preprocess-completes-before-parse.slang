//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#lex--preprocess
//META: doc_section_digest=5c3df6c5ee061f965c288cc9fd7cc0bfdc08b911677308aa464a6cefdae4a23c
//META: purpose=Macro expansion and conditional compilation finish before parsing, so only the selected, fully expanded token text reaches the parser and every downstream stage.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that preprocessing is complete before the parser runs, observed
// from the far end of the pipeline. Three preprocessor features feed one
// expression: an object-like macro, a function-like macro that uses it, and
// an `#if` that selects between two tag values. Each target's CHECK pins the
// expanded operands and the tag from the taken arm; the discarded arm's tag
// never appears anywhere in the artefact, which is only possible if the
// untaken branch's tokens were dropped before the parser -- and hence before
// checking, lowering, and emit -- ever saw them.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

#define TILE 8
#define SCALED(x) ((x) * TILE)

#if TILE > 4
#define TAG 1110
#else
#define TAG 2220
#endif

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = SCALED(int(tid.x)) + TAG;
}

// HLSL: int(8) + int(1110)
// HLSL-NOT: 2220

// GLSL: * 8 + 1110
// GLSL-NOT: 2220

// SPIRV-DAG: %int_8 = OpConstant %int 8
// SPIRV-DAG: %int_1110 = OpConstant %int 1110
// SPIRV-DAG: OpIMul %int
// SPIRV-DAG: OpIAdd %int

// METAL: int(8) + int(1110)
// METAL-NOT: 2220

// WGSL: i32(8) + i32(1110)
// WGSL-NOT: 2220

// CUDA: int(8) + int(1110)
// CUDA-NOT: 2220

// CPP: int(8) + int(1110)
// CPP-NOT: 2220
