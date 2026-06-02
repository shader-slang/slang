//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=Two `RWStructuredBuffer<int>` resources emit as independent StorageBuffer variables with distinct SPIRV Binding decorations; the doc states that if the client API binds them to the same underlying memory they are aliased and concurrent non-atomic access constitutes a data race.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: if the same underlying memory is bound to multiple resources
// such as RWStructuredBuffer<T>, it is aliased. A data race can occur even when
// two concurrent accesses use different buffer handles, when they modify
// overlapping underlying memory non-atomically without happens-before.
// This emission test pins that two separate RWStructuredBuffer bindings are
// emitted as independent StorageBuffer variables, each with its own Binding.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> bufA;
RWStructuredBuffer<int> bufB;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // if bufA and bufB alias the same underlying memory, the write to bufA[0]
    // and the read of bufB[0] constitute a data race per the doc's definition
    bufA[0] = 42;
    int val = bufB[0];
    bufA[1] = val;
}

// CHECK-DAG: OpDecorate %bufA{{.*}} Binding
// CHECK-DAG: OpDecorate %bufB{{.*}} Binding
// CHECK-DAG: OpVariable {{.*}} StorageBuffer
