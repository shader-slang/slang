//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#overloadedexpr-and-overloadedexpr2
//META: doc_section_digest=9895bfca5ddc67f2f49caef21aa6068a74e3319decff398b32f9bbfc36767520
//META: purpose=Verifies that a call whose argument matches two candidates equally well is rejected with an ambiguity diagnostic naming both candidates.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Failing to collapse the candidate set has two distinct outcomes: no
// applicable candidate, or more than one equally good candidate. This test
// covers the second: a `double` argument converts to both the `int` and the
// `float` overload at the same cost. The CHECKs pin the ambiguity error plus
// the two candidate notes the checker attaches.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry notMain -stage compute

void f(int x) { }
/*CHECK:
     ^ candidate: func f(int) -> void
*/
void f(float x) { }
/*CHECK:
     ^ candidate: func f(float) -> void
*/

void notMain()
{
    f(1.0lf);
/*CHECK:
     ^ ambiguous call to 'f' with arguments of type (double)
*/
}
