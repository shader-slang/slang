//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=c1ef039f2fd0378a7cac58b8bb8282b937d992edac625ada2b1761464ceb475b
//META: purpose=The generic typeLayout opcode carries only attribute operands, with no fixed layout operand ahead of them.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the fallback `typeLayout` record has no fixed operand of its
// own -- every operand is an attribute. The element type of the structured
// buffer gets one; the test captures both of its operands and asserts that the
// first resolves to a `size` attribute and the second to a `TypeAlignment`
// attribute, which is only true if the attribute tail starts at operand 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: %[[TL:[0-9]+]]{{.*}}= typeLayout(%[[SZ:[0-9]+]], %[[AL:[0-9]+]])
// CHECK: %[[AL]]{{.*}}= TypeAlignment(
// CHECK: %[[SZ]]{{.*}}= size(
