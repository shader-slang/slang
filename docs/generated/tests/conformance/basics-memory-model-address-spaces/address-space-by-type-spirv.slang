//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=The address space is determined by type, modifiers, and attributes simultaneously; a single shader mixing multiple address spaces emits each variable into its distinct SPIRV storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that type, modifiers, and attributes jointly determine address space, so a single
// shader can hold variables in four distinct address spaces at once. A ConstantBuffer (Uniform),
// RWStructuredBuffer (StorageBuffer), groupshared array (Workgroup), and dispatch-thread-ID
// parameter (Input) all coexist; CHECK-DAG asserts each maps to its own SPIRV storage class.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

// Uniform address space (ConstantBuffer<T> -> Uniform).
ConstantBuffer<float> cbParam;

// Storage buffer address space (RWStructuredBuffer<T> -> StorageBuffer).
RWStructuredBuffer<float> rwbuf;

// Group-shared address space (static groupshared -> Workgroup).
static groupshared float lds[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)  // Input address space -> Input.
{
    lds[tid.x] = cbParam + float(tid.x);
    GroupMemoryBarrierWithGroupSync();
    rwbuf[tid.x] = lds[3u - tid.x];
}

// Each address space maps to its own SPIRV storage class (order-independent).
// CHECK-DAG: OpVariable {{.*}} Uniform
// CHECK-DAG: OpVariable {{.*}} StorageBuffer
// CHECK-DAG: OpVariable {{.*}} Workgroup
// CHECK-DAG: OpVariable {{.*}} Input
