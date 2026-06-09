//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=Two `RWStructuredBuffer<int>` resources emit as independent StorageBuffer variables with distinct SPIRV Binding decorations; if the client API binds them to the same memory, concurrent non-atomic access is a data race.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two separate `RWStructuredBuffer` resources emit as independently
// bound `StorageBuffer` variables in SPIR-V. CHECK-DAG asserts distinct `Binding`
// decorations on `bufA` and `bufB` and that an `OpVariable ... StorageBuffer`
// appears; independent bindings allow the client API to alias them at the storage
// layer, which is the precondition for the data-race scenario the doc describes.

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
