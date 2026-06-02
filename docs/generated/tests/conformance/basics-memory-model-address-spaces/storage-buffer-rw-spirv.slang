//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`RWStructuredBuffer<T>` and `StructuredBuffer<T>` belong to the Storage buffer address space; SPIRV emits both with `StorageBuffer` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that both `RWStructuredBuffer<T>` and `StructuredBuffer<T>` belong to the
// Storage buffer address space and share the `StorageBuffer` SPIRV storage class.
// CHECK-DAG asserts `%rwbuf` emits as `OpVariable ... StorageBuffer` and `%robuf` emits
// similarly with a `NonWritable` decoration, distinguishing read-write from read-only.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> rwbuf;
StructuredBuffer<float>   robuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    float v = robuf[0];
    rwbuf[0] = v + 1.0;
    output[0] = rwbuf[0];
}

// Both read-write and read-only structured buffers map to StorageBuffer storage class.
// CHECK-DAG: %rwbuf = OpVariable {{.*}} StorageBuffer
// CHECK-DAG: %robuf = OpVariable {{.*}} StorageBuffer {{.*}} NonWritable
