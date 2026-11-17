//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=493f392639c5a17ee3f1baf8a77a2222a0f4ff648b2f59628575efc1dd1a87c3
//META: purpose=Negative: bare `-target metal` stops at the Metal text artifact -- no Apple `metal` compile and no `.metallib` step run.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the downstream Apple toolchain is reserved for the
// `MetalLib*` targets: a bare `-target metal` request must produce
// human-readable MSL and nothing else, which is why this bundle can run
// without Xcode at all. CHECK asserts the output is source text starting
// at the prelude and containing the renamed entry point, and that no
// HLSL-style `void main(` signature is present.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK: void main_0(
// CHECK-NOT: void main(
