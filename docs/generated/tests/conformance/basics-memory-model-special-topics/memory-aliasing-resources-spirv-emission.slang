//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=`ConstantBuffer<T>` and `RWStructuredBuffer<T>` emit as independently-decorated SPIRV variables (distinct Binding decorations, Uniform vs StorageBuffer storage classes); they are never merged even when the client API could bind them to overlapping memory.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `ConstantBuffer<T>` and `RWStructuredBuffer<T>` emit as
// independently-decorated variables in SPIR-V: `cbuf` gets Uniform storage
// class and `rwbuf` gets StorageBuffer storage class, each with its own
// Binding decoration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

ConstantBuffer<int> cbuf;
RWStructuredBuffer<int> rwbuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int val = cbuf;
    rwbuf[0] = val;
}

// ConstantBuffer emits as Uniform storage class with a Block struct
// CHECK-DAG: OpDecorate %cbuf{{.*}} Binding
// CHECK-DAG: OpDecorate %cbuf{{.*}} DescriptorSet
// CHECK-DAG: OpVariable %_ptr_Uniform_{{.*}} Uniform
// RWStructuredBuffer emits as StorageBuffer storage class
// CHECK-DAG: OpDecorate %rwbuf{{.*}} Binding
// CHECK-DAG: OpDecorate %rwbuf{{.*}} DescriptorSet
// CHECK-DAG: OpVariable %_ptr_StorageBuffer_{{.*}} StorageBuffer
