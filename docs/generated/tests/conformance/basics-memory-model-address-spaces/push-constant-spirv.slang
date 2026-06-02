//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`[vk::push_constant]`-annotated struct belongs to the Push constant address space; SPIRV emits it with `PushConstant` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a struct annotated with `[[vk::push_constant]]` belongs to the Push constant
// address space. The shader reads two fields and writes their product to an output buffer so
// the variable is not DCE'd; CHECK asserts the SPIRV declares `%pushConst` as
// `OpVariable ... PushConstant`, confirming the attribute-to-storage-class mapping.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

struct PushData {
    float scale;
    int   count;
};

[[vk::push_constant]]
PushData pushConst;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = pushConst.scale * float(pushConst.count);
}

// CHECK: %pushConst = OpVariable {{.*}} PushConstant
