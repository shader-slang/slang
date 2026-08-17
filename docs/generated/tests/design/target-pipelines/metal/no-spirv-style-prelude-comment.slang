//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=f18a62d46309fae1d33b5fed992f19c2eff74d722d039dfeeeccc0dfdbf5b824
//META: purpose=Negative: the Metal source emitter writes no target-identification header comment; the first emitted text is the `#include` block.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Structural negative observation about the Metal prelude: unlike the
// SPIR-V assembly writer, `MetalSourceEmitter` prepends no `; SPIR-V` /
// `; Version` style banner, so tests and downstream tools can rely on the
// first non-blank line being a preprocessor directive. CHECK anchors on
// the first include and then asserts neither banner token appears anywhere
// in the output.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK-NOT: SPIR-V
// CHECK-NOT: ; Version
