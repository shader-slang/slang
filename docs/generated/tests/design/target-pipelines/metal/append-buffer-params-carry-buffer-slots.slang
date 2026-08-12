//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=0c4ba248635633393b51ae889f226e3b5f86a549ad5be4af6f827414819fb357
//META: purpose=Regression: the element and counter pointers synthesized for an `AppendStructuredBuffer` must each carry a positional `[[buffer(N)]]` slot like every other Metal kernel resource parameter.
//META: intent=regression
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Every resource parameter of a Metal kernel needs a binding-slot
// attribute -- a parameter emitted without one has no way for the host to
// bind it, and the Apple compiler rejects the source. This test takes the
// two pointer parameters that `lowerAppendConsumeStructuredBuffers`
// synthesizes for an append buffer and asserts each is followed by a
// `buffer(` marker, the same way the plain `RWStructuredBuffer` parameter
// beside it is.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

AppendStructuredBuffer<float> appendBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    appendBuf.Append(float(tid.x));
    outBuf[tid.x] = 1.0f;
}

// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: appendBuf_elements_{{[0-9]+}} {{\[\[}}buffer(
// CHECK-SAME: appendBuf_counter_{{[0-9]+}} {{\[\[}}buffer(
