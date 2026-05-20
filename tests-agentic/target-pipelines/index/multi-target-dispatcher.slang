//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:33Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/target-pipelines/index.md#target-pipelines
//META: doc_section_digest=21f553cd71c30908d012f7fa83b90c4d5d11d21b85cd9e8ad32892129e15a2fa
//META: purpose=A single Slang source dispatches through the multi-target orchestrator to each of the five text-emit backends named by the index doc (spirv, hlsl, metal, wgsl, cuda).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index page lists five per-target pages -- spirv, hlsl, metal,
// wgsl, cuda -- as the targets dispatched by the shared orchestrator
// linkAndOptimizeIR. The cross-cutting observable is that one source
// reaches successful emission on all five text-emit targets. We assert
// only that each backend produced its characteristic preamble / shape,
// not the per-target details (those belong to peer bundles).

//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + 7;
}

// SPIR-V direct emit shape (Khronos asm header).
// CHECK_SPV: SPIR-V
// CHECK_SPV: OpEntryPoint GLCompute

// HLSLSourceEmitter shape.
// CHECK_HLSL: numthreads(1, 1, 1)
// CHECK_HLSL: void main

// MetalSourceEmitter shape (metal stdlib + kernel keyword).
// CHECK_METAL: metal_stdlib
// CHECK_METAL: kernel

// WGSLSourceEmitter shape (compute attribute + workgroup_size).
// CHECK_WGSL: @compute
// CHECK_WGSL: @workgroup_size(1, 1, 1)

// CUDASourceEmitter shape (global kernel via CPP base emitter).
// CHECK_CUDA: __global__
