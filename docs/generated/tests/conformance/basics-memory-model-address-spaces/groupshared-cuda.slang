//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`static groupshared` belongs to the Group-shared address space; CUDA emits it as `__device__ __shared__`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table row for "Group-shared" states: `static groupshared` (global scope)
// is in the Group-shared address space, instance scope = thread group.
// On CUDA, group-shared variables map to the `__shared__` memory qualifier.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

static groupshared float sharedArr[8];

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    sharedArr[tid.x] = float(tid.x) * 3.0;
    GroupMemoryBarrierWithGroupSync();
    output[tid.x] = sharedArr[7u - tid.x];
}

// CUDA group-shared maps to __shared__ device memory.
// CHECK: __shared__
// CHECK-DAG: sharedArr{{.*}}
