//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#end-to-end-flow
//META: doc_section_digest=1ecebc97d9b9a7bc9f7446de832f668a21384d423d0e4828c931dfa6ab8d61e8
//META: purpose=One non-trivial source traverses lex, preprocess, parse, check, lower, IR passes, and emit and emerges as a recognizable artefact on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the whole source-to-artefact data flow runs to completion,
// on every text-emit back-end, for a source that needs each stage to do
// real work: a macro (preprocess), a struct decl and a callee (parse +
// check + lower), a branch (basic-block formation), and a buffer write
// (layout + legalization). Each target's CHECK-DAG set pins one artefact
// per stage, so a stage that silently dropped its contribution would leave
// its marker missing from the emitted text rather than producing a
// superficially valid file.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

#define LIMIT 4

struct Counter
{
    int value;
};

int bump(Counter c)
{
    return c.value + LIMIT;
}

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Counter c;
    c.value = int(tid.x);
    int next = bump(c);
    if (next > 0)
        gOut[0] = next;
    else
        gOut[0] = -next;
}

// The macro is expanded before parsing, so the literal 4 -- and never the
// spelling LIMIT -- reaches the emitted text on every target.
// HLSL-DAG: struct Counter
// HLSL-DAG: int bump_{{[0-9]+}}(
// HLSL-DAG: numthreads(1, 1, 1)
// HLSL-DAG: void computeMain(
// HLSL-DAG: int(4)
// HLSL-NOT: LIMIT

// GLSL-DAG: struct Counter
// GLSL-DAG: int bump_{{[0-9]+}}(
// GLSL-DAG: local_size_x = 1
// GLSL-DAG: + 4
// GLSL-NOT: LIMIT

// SPIR-V force-inlines the callee, so the callee's contribution shows up as
// the folded constant and the branch rather than as a separate function.
// SPIRV-DAG: OpEntryPoint GLCompute
// SPIRV-DAG: OpExecutionMode {{.*}} LocalSize 1 1 1
// SPIRV-DAG: %int_4 = OpConstant %int 4
// SPIRV-DAG: OpBranchConditional

// METAL-DAG: struct Counter
// METAL-DAG: int bump_{{[0-9]+}}(
// METAL-DAG: {{\[\[}}kernel{{\]\]}} void computeMain(
// METAL-DAG: int(4)

// WGSL-DAG: struct Counter
// WGSL-DAG: fn bump_{{[0-9]+}}(
// WGSL-DAG: @workgroup_size(1, 1, 1)
// WGSL-DAG: i32(4)

// CUDA-DAG: struct Counter
// CUDA-DAG: __device__ int bump_{{[0-9]+}}(
// CUDA-DAG: __global__ void computeMain(
// CUDA-DAG: int(4)

// CPP-DAG: struct Counter
// CPP-DAG: bump_{{[0-9]+}}(
// CPP-DAG: SLANG_PRELUDE_EXPORT
// CPP-DAG: int(4)
