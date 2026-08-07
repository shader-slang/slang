//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=atomicAdd at uint MAX (0xFFFFFFFFu) emits the value as u32(4294967295) inside the atomicAdd call
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Combined boundary: atomicAdd carries the unsigned MAX literal as
// its 'amount' operand, which the WGSL emitter prints as
// u32(4294967295) inside the atomicAdd argument list.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<Atomic<uint>> ctr;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint old = ctr[0].add(0xFFFFFFFFu);
    outBuf[tid.x] = old;
}

// CHECK: atomicAdd
// CHECK-SAME: u32(4294967295)
