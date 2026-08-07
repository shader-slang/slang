//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#usersemantic-vs-systemvaluesemantic
//META: doc_section_digest=5d4c2952398382bdc9ae3befb591dfdbf6629429f5af8c13daa49fd77c0de113
//META: purpose=A varLayout carries at most one semantic attribute: an SV_ parameter gets a systemValueSemantic and no userSemantic is created for it.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "at most one semantic" rule and which side wins. Var-layout
// lowering checks the system-value semantic first and only falls back to the
// user semantic, even though the front-end record can hold both. The CHECK
// pins the system-value attribute for `SV_DispatchThreadID` and the CHECK-NOT
// asserts no `userSemantic` naming an `SV_` string was also emitted.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = systemValueSemantic("SV_DispatchThreadID", {{[0-9]+}} : Int)
// CHECK-NOT: userSemantic("SV_
