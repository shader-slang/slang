//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=OverloadGroupType is the pseudo-type of an unresolved overload set; when overload resolution is ambiguous the diagnostic reports the candidate set.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable surface of OverloadGroupType: two overloads
// of `f` are equally good for (int, int) (each needs one int->float
// conversion), so the unresolved overload set cannot collapse and the
// checker reports an ambiguous call over the candidate set. CHECK
// pins the ambiguity diagnostic and both candidate notes.

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
