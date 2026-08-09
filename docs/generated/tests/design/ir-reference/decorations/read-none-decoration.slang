//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#inlining-and-optimization
//META: doc_section_digest=920c2eb81b315af907f28c1ce483b160e67a9a325de3473075b6cc826161fb7b
//META: purpose=Boundary - a function carrying [__readNone] surfaces a bare readNone decoration distinct from noSideEffect and the other inlining-and-optimization opcodes.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `readNone` is its own marker rather than a synonym of
// `noSideEffect`: the same helper shape as the `noSideEffect` test is used
// and only the attribute changes, so the CHECK on `[readNone]` distinguishes
// the two rows. The user-visible spelling is `[__readNone]`; the doc's
// `[__readNone]` naming is what this test exercises.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[__readNone]
int pureFn(int x) { return x * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pureFn(int(tid.x));
}

// CHECK: [readNone]
// CHECK: [nameHint("pureFn")]
// CHECK: func %pureFn
