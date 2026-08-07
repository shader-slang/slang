//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=d2bb343d99876dfd1d05e29a3d730266a1a5c82ccace506601773d10ac4885df
//META: purpose=A function declaration in the AST becomes an IR function that reaches emit, appearing as a separate callee on the source back-ends and as an inlined body on SPIR-V.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the decl-to-IR-function half of lowering, and incidentally that
// the pass list downstream of it is target-sensitive. The callee takes two
// dispatch-derived arguments so the constant folder cannot evaluate it
// away. On the C-family and WGSL back-ends the CHECKs pin a standalone
// definition plus the call site; on SPIR-V the same IR function is
// force-inlined by the target-specific passes, so the CHECKs instead pin
// the entry point's own function plus the arithmetic that the callee body
// contributed, and assert that no call instruction remains.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

int helperFn(int a, int b)
{
    return a * b + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = helperFn(int(tid.x), int(tid.y));
}

// HLSL-DAG: int helperFn_{{[0-9]+}}(int
// HLSL-DAG: = helperFn_{{[0-9]+}}(

// GLSL-DAG: int helperFn_{{[0-9]+}}(int
// GLSL-DAG: = helperFn_{{[0-9]+}}(

// SPIRV-DAG: OpFunction %void None
// SPIRV-DAG: OpIMul %int
// SPIRV-DAG: OpIAdd %int
// SPIRV-NOT: OpFunctionCall

// METAL-DAG: int helperFn_{{[0-9]+}}(int
// METAL-DAG: helperFn_{{[0-9]+}}(

// WGSL-DAG: fn helperFn_{{[0-9]+}}(
// WGSL-DAG: = helperFn_{{[0-9]+}}(

// CUDA-DAG: __device__ int helperFn_{{[0-9]+}}(int
// CUDA-DAG: = helperFn_{{[0-9]+}}(

// CPP-DAG: helperFn_{{[0-9]+}}(int32_t
// CPP-DAG: = helperFn_{{[0-9]+}}(
