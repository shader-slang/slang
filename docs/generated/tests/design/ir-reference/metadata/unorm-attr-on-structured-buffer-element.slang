//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=0e7a8ec2a490ce368dde09c10d45863e67fc1e491cc433074fe4f7bbf890e750
//META: purpose=The unorm modifier on a structured-buffer element type produces a no-operand unorm attribute inside an Attributed type wrapper.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same UNORM marker attribute on the other carrier the modifier
// is allowed on: a structured-buffer element rather than a texture element.
// The CHECK captures the zero-operand attribute definition and requires the
// same id to be the attribute operand of an `Attributed` type, so a regression
// that only handled the texture carrier would show up here.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<unorm float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 0.5;
}

// CHECK: %[[U:[0-9]+]]{{.*}}= unorm
// CHECK: Attributed({{.*}}, %[[U]])
