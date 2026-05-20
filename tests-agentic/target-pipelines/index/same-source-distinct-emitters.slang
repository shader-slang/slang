//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:33Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/target-pipelines/index.md#cross-target-comparison
//META: doc_section_digest=2189df77247b3c5793c79496940422c4d89b7ff54728e5b6402f4c854362eece
//META: purpose=The cross-target table maps each target to a distinct Phase D emitter; the same Slang source therefore produces distinctively different surface syntax on each backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc's cross-target comparison table lists per-target
// emitters: HLSLSourceEmitter, MetalSourceEmitter, WGSLSourceEmitter,
// CUDASourceEmitter, and the SPIR-V direct-emit path. The
// cross-cutting observable for "Phase D divergence" is that the same
// Slang declaration of an RWStructuredBuffer comes out as
// markedly different surface text on each target: register(u0)
// on HLSL, OpDecorate Binding 0 / DescriptorSet 0 on SPIR-V,
// [[buffer(N)]] on Metal, @binding/@group on WGSL, and a templated
// RWStructuredBuffer<int> on CUDA.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: register(uN) binding spelling.
// CHECK_HLSL: register(u{{[0-9]+}})

// SPIR-V: OpDecorate Binding / DescriptorSet.
// CHECK_SPV: OpDecorate {{.*}} Binding 0
// CHECK_SPV: OpDecorate {{.*}} DescriptorSet 0

// Metal: [[buffer(N)]] positional attribute. (Use a bare substring;
// FileCheck treats [[...]] as a regex-var reference.)
// CHECK_METAL: buffer(0)

// WGSL: @binding / @group binding spelling.
// CHECK_WGSL: @binding(0)
// CHECK_WGSL: @group(0)

// CUDA: the templated RWStructuredBuffer<int> survives into CUDA text.
// CHECK_CUDA: RWStructuredBuffer<int>
