//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`[vk::specialization_constant]`-annotated variables belong to the Specialization constant address space; SPIRV emits them as `OpSpecConstant` with a `SpecId` decoration.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that constants annotated with `[[vk::specialization_constant]]` or
// `[[SpecializationConstant]]` land in the Specialization constant address space.
// Unlike Uniform variables, specialization constants emit as `OpSpecConstant` (not `OpVariable`)
// each with a `SpecId` decoration; CHECK-DAG confirms both opcodes are present for both
// attribute spellings.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

[[vk::specialization_constant]]
const int WORK_SIZE = 4;

[[SpecializationConstant]]
const float SCALE_FACTOR = 1.5;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = float(WORK_SIZE) * SCALE_FACTOR;
}

// Both attributes produce OpSpecConstant entries with SpecId decorations (order-independent).
// CHECK-DAG: OpSpecConstant %int {{.*}}
// CHECK-DAG: OpSpecConstant %float {{.*}}
// CHECK-DAG: OpDecorate {{.*}} SpecId
