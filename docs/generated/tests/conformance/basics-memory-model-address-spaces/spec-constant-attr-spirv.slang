//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`[SpecializationConstant]` (non-vk form) also belongs to the Specialization constant address space; SPIRV emits it as `OpSpecConstant` with `SpecId` decoration.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `[[SpecializationConstant]]` attribute (the non-vk-namespaced form)
// also routes a constant to the Specialization constant address space. CHECK-DAG asserts
// the SPIRV output contains both `OpSpecConstant` and an `OpDecorate ... SpecId` decoration,
// proving both attribute spellings produce the same encoding as `[[vk::specialization_constant]]`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

[[SpecializationConstant]]
const int TILE_SIZE = 8;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<int> output)
{
    output[0] = TILE_SIZE * TILE_SIZE;
}

// Both OpDecorate SpecId and OpSpecConstant must appear (order-independent).
// CHECK-DAG: OpSpecConstant %int {{.*}}
// CHECK-DAG: OpDecorate {{.*}} SpecId
// Metal maps [SpecializationConstant] to function_constant with a default fallback.
// METAL: function_constant(
// METAL-DAG: is_function_constant_defined(
// WGSL maps [SpecializationConstant] to an override declaration with @id.
// WGSL: @id(
// WGSL-DAG: override TILE_SIZE_
