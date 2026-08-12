//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=An unresolved overload set is a pseudo-type that checking must collapse; when no candidate is better the call is rejected and the diagnostic names the candidate set.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable surface of the unresolved-overload pseudo-
// type: both `f` overloads need exactly one int-to-float conversion for
// the call `f(1, 2)`, so neither wins and the set cannot collapse.
// CHECK pins the ambiguity diagnostic plus one note per candidate,
// which shows the set was carried to the point of resolution.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f(int x, float y) { return 1; }
/*CHECK:
    ^ candidate: func f(int, float) -> int
*/
int f(float x, int y) { return 2; }
/*CHECK:
    ^ candidate: func f(float, int) -> int
*/

void main()
{
    int r = f(1, 2);
//CHECK:     ^ ambiguous call to 'f' with arguments of type (int, int)
}
