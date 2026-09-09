//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=1d8e7a1c30049e4d9b67c4beb3ca230fe889d9d80802b14075e9c38d36c53d4c
//META: purpose=lowerAppendConsumeStructuredBuffers runs for WGSL and splits AppendStructuredBuffer<T> into an atomic counter buffer plus an element buffer
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Phase B `lowerAppendConsumeStructuredBuffers` pass fires
// for WGSL (its `target != HLSL` arm is true here). The single Slang-level
// `AppendStructuredBuffer<uint>` global has no WGSL equivalent, so the pass
// must replace it with two plain storage buffers -- an `atomic` counter and
// an element array -- and rewrite `.Append(v)` into an `atomicAdd` on the
// counter followed by a store into the element array. CHECK pins both
// generated globals and the atomicAdd, which only exist if the pass ran.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;
AppendStructuredBuffer<uint> appBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    appBuf.Append(tid.x);
    outBuf[tid.x] = tid.x;
}

// CHECK-DAG: appBuf_counter_{{[0-9]+}} : array<atomic<i32>>
// CHECK-DAG: appBuf_elements_{{[0-9]+}} : array<u32>
// CHECK: atomicAdd
