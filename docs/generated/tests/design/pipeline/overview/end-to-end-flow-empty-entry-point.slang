//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#end-to-end-flow
//META: doc_section_digest=1ecebc97d9b9a7bc9f7446de832f668a21384d423d0e4828c931dfa6ab8d61e8
//META: purpose=The smallest possible program -- a single entry point with an empty body and no parameters -- still traverses the whole pipeline and produces a target artefact.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower edge of the end-to-end data flow: with no macros, no
// types, no parameters and no statements, every stage has the minimum
// possible amount of work to do, and the emit stage still has to produce a
// complete artefact. CHECK asserts that each back-end emits a fully formed
// entry point (signature plus the explicit `return` the lowering pass
// synthesizes for the empty body), which is only reachable if the empty
// AST body survived lowering as a single basic block rather than being
// discarded.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}

// HLSL: void computeMain()
// HLSL: return;

// GLSL: void main()
// GLSL: return;

// SPIRV-DAG: OpEntryPoint GLCompute
// SPIRV-DAG: OpFunction %void None
// SPIRV-DAG: OpReturn
// SPIRV-DAG: OpFunctionEnd

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain()
// METAL: return;

// WGSL: fn computeMain()
// WGSL: return;

// CUDA: __global__ void computeMain()
// CUDA: return;

// CPP: void _computeMain(
// CPP: return;
// CPP: SLANG_PRELUDE_EXPORT
