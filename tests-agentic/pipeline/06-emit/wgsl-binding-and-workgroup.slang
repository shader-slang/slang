//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#wgsl
//META: doc_section_digest=c4d36907ae85aa70fabbf2ec79508a8a9a447cb63c069d92f6f276441747833c
//META: purpose=The WGSL backend emits WGSL source with @binding(N) @group(N) for resources and @compute / @workgroup_size for the compute entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The WGSL backend (slang-emit-wgsl.{h,cpp}) emits WGSL: resources
// are annotated with @binding(N) and @group(N), and a compute entry
// point is annotated with @compute and @workgroup_size(...). A
// vk::binding annotation in source picks up the binding and group
// indices for WGSL too.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

[vk::binding(2, 3)]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(16, 2, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: @binding(2) @group(3){{.*}}var<storage, read_write> outBuf
// CHECK: @compute
// CHECK: @workgroup_size(16, 2, 1)
// CHECK: fn main(
