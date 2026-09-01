//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#loop-and-branch-hints
//META: doc_section_digest=647327ee76a1bc33d8bcf7d2abc19a2f16ee4e63d10bad4d1d5d48c120029d9d
//META: purpose=Negative - [ForceUnroll] on a function declaration is rejected with E31002 because the attribute is valid only on a loop statement.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the placement restriction implied by `ForceUnroll` being a
// loop-control decoration: written on a function declaration the attribute
// never reaches lowering. The caret annotation pins the attribute span and
// the code CHECK pins E31002, so both the position and the identity of the
// diagnostic are asserted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main
uniform RWStructuredBuffer<int> buf;

[ForceUnroll]
/*CHECK:
 ^^^^^^^^^^^ attribute 'ForceUnroll' is not valid here
*/
// CHECK: E31002
int helper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(int(tid.x));
}
