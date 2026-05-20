//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:33Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/target-pipelines/index.md#cross-target-comparison
//META: doc_section_digest=2189df77247b3c5793c79496940422c4d89b7ff54728e5b6402f4c854362eece
//META: purpose=The text-emit CodeGenTarget rows in the cross-target table (hlsl, metal, wgsl, cuda source, spirv-asm) reach Phase D source without invoking downstream tools.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The cross-target comparison table separates source-text targets
// (the per-target SourceEmitters / SPIR-V direct emit) from
// downstream-binary targets (DXIL, DXBytecode, MetalLib, WGSLSPIRV,
// PTX) gated on external compilers (DXC, fxc, Apple `metal`, Tint,
// nvrtc). The cross-cutting consequence on a no-GPU runner is that
// the five text-emit targets named in the index doc compile cleanly
// to source / asm without any of the downstream tools running. We
// FileCheck a per-target source-level marker (no DXIL magic bytes,
// no metal-lib container header, no SPIR-V binary header on
// spirv-asm) to confirm Phase D stopped at text.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV_ASM):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL text emit: pragma + register syntax appear in source, not
// post-DXC DXIL bitcode.
// CHECK_HLSL: pragma pack_matrix
// CHECK_HLSL: register

// Metal text emit: metal_stdlib header appears in source, not a
// MetalLib binary container.
// CHECK_METAL: metal_stdlib

// WGSL text emit: @compute attribute appears in source, not a
// Tint-produced SPIR-V binary.
// CHECK_WGSL: @compute

// CUDA text emit: the prelude include appears in source, not a
// nvrtc-produced PTX binary.
// CHECK_CUDA: slang-cuda-prelude

// SPIR-V asm: the textual asm preamble appears, not the binary
// SPIR-V magic + bound header.
// CHECK_SPV_ASM: SPIR-V
// CHECK_SPV_ASM: OpMemoryModel
