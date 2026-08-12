//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#downstream-apple-metal-compiler
//META: doc_section_digest=65e900d0cbf2c6b34b856edc81e93bce49e3fc22fad6946b0d7bc5d0b4974910
//META: purpose=`-target metallib` hands the emitted MSL to Apple's `metal` compiler (the `MetalC` pass-through) to produce a `.metallib` carrying the renamed entry point.
//META: intent=expansion
//META: pipeline_stage=runtime
//META: requires-tool=metal-toolchain
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `Metal` -> `MetalLib` transition end to end. Slang itself
// does no MSL grammar checking, so a successful `.metallib` is also the
// only signal that the text it emitted is actually valid Metal. CHECK
// asserts the resulting bytecode still defines the Slang-renamed kernel.
// The Apple toolchain is required, so runners without it report this
// ignored and CI validates it.

//TEST:SIMPLE(filecheck=CHECK):-target metallib -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) + 1;
}

// CHECK: define
// CHECK-SAME: main_0
