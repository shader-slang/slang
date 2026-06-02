//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=The `[push_constant]` attribute (non-vk-prefixed form) also belongs to the Push constant address space; SPIRV emits it with `PushConstant` storage class.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table row for "Push constant" lists TWO attribute spellings:
// `[push_constant]` and `[vk_push_constant]`.
// The primary smoke test (push-constant-spirv.slang) uses `[vk::push_constant]`.
// This boundary test verifies the `[push_constant]` spelling also produces
// PushConstant storage class in SPIRV.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

struct SmallData {
    float x;
    float y;
};

[[push_constant]]
SmallData pc;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = pc.x + pc.y;
}

// CHECK: %pc = OpVariable {{.*}} PushConstant
