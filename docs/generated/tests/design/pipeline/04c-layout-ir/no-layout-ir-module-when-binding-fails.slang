//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#when-it-is-built
//META: doc_section_digest=25e99ccbf617cd9d84684ad01c0587f61ae6261d821b4f8dc158b862abdd9cb1
//META: purpose=When parameter binding fails with an error no layout IR module is built, so the dump contains no layout instructions after the pre-link block.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the error gate in front of layout-IR construction. The miss
// entry point's payload has no layout rules on Metal, so binding reports an
// error and returns no program layout. CHECK confirms the pre-link block is
// still produced, and the CHECK-NOTs confirm no layout instruction or
// layout decoration exists anywhere in the dump - construction never ran.

//TEST:SIMPLE(filecheck=CHECK):-target metal -dump-ir -o /dev/null -entry main -stage miss

struct Payload
{
    float4 color;
}

[shader("miss")]
void main(inout Payload payload)
{
    payload.color = float4(1, 0, 0, 1);
}

// CHECK: ### LOWER-TO-IR:
// CHECK-NOT: EntryPointLayout(
// CHECK-NOT: structTypeLayout(
// CHECK-NOT: parameterGroupTypeLayout(
// CHECK-NOT: [layout(
// CHECK: error{{.*}}39032
