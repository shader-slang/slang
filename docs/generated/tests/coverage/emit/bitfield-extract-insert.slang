//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies `bitfieldExtract`/`bitfieldInsert` lower to native ops on SPIR-V and GLSL but to expanded shift/mask arithmetic on HLSL.
//META: intent=characterization
//META: covers=source/slang/slang-emit-spirv.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the per-target divergence of the bitfield intrinsics. SPIR-V emits
// the native `OpBitFieldUExtract` / `OpBitFieldInsert` (slang-emit-spirv.cpp);
// GLSL emits the built-in `bitfieldExtract` / `bitfieldInsert` with int
// casts on the offset/bits operands (slang-emit-glsl.cpp); HLSL has no
// intrinsic so the c-like emitter lowers to explicit shift/mask
// arithmetic. The operands are derived from tid.x (runtime) so the
// intrinsic is not constant-folded, and the result is sunk to outputBuffer.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint v = tid.x | 0xF0u;
    uint e = bitfieldExtract(v, 4u, 4u);
    uint i = bitfieldInsert(v, 0x3u, 0u, 2u);
    outputBuffer[0] = e + i;
}

// HLSL has no bitfield intrinsic; it emits explicit shift/mask arithmetic.
//HLSL-DAG: >>(4U))&
//HLSL-DAG: ~(((uint(1)<<2U)-uint(1))
// GLSL uses the GLSL built-ins, casting offset/bits to int.
//GLSL-DAG: bitfieldExtract(
//GLSL-DAG: bitfieldInsert(
// SPIR-V uses the native bitfield opcodes.
//SPV-DAG: OpBitFieldUExtract
//SPV-DAG: OpBitFieldInsert
// Metal has no bitfield intrinsic either; same expanded shift/mask form.
//METAL-DAG: >>(4U))&
//METAL-DAG: ~(((uint32_t(1)<<2U)-uint32_t(1))
// WGSL also expands to shift/mask arithmetic (u32-typed).
//WGSL-DAG: v_0>>(u32(4))
//WGSL-DAG: ~(((u32(1)<<u32(2))-u32(1))
