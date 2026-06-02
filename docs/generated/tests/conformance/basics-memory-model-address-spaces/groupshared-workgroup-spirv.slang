//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`static groupshared` at global scope belongs to the Group-shared address space; SPIRV emits it with `Workgroup` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `static groupshared` at global scope maps to the Workgroup storage class in
// SPIRV. The kernel reverses values through `sharedData`; CHECK asserts the SPIRV assembly
// declares the array as `OpVariable ... Workgroup`, confirming the thread-group address space.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

static groupshared float sharedData[32];

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    sharedData[tid.x] = float(tid.x);
    GroupMemoryBarrierWithGroupSync();
    output[tid.x] = sharedData[31u - tid.x];
}

// CHECK: %sharedData = OpVariable {{.*}} Workgroup
