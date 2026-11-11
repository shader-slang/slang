//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=a11a177f08e66b89cc22b72f8789aac183281f455fe9b5f1596303bdf81cd2e2
//META: purpose=Atomic<int> emits as atomic<i32> and .add() lowers to atomicAdd on the i32 element
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The signed-int side of the atomic<T> axis: the element type
// switches from u32 to i32 while the atomicAdd intrinsic shape is
// preserved.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<Atomic<int>> ctr;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int old = ctr[0].add(1);
    outBuf[tid.x] = old;
}

// CHECK: array<atomic<i32>
// CHECK: atomicAdd
