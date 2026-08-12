//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=920c2eb81b315af907f28c1ce483b160e67a9a325de3473075b6cc826161fb7b
//META: purpose=Stress - a single function carrying many compatible inlining-and-optimization decorations produces every corresponding decoration in the dump, with no interference between them.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the inlining/optimization decorations are independent
// markers that accumulate on one inst. Four attributes are stacked on a
// single helper and the CHECK-DAG lines assert `ForceInline`,
// `noSideEffect`, `readNone` and `unsafeForceInlineEarly` all appear
// alongside the helper's `nameHint`, which anchors the match at user code.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[ForceInline]
[__NoSideEffect]
[__readNone]
[__unsafeForceInlineEarly]
int superHelper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = superHelper(int(tid.x));
}

// CHECK-DAG: [ForceInline]
// CHECK-DAG: [noSideEffect]
// CHECK-DAG: [readNone]
// CHECK-DAG: [unsafeForceInlineEarly]
// CHECK-DAG: [nameHint("superHelper")]
