//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=5e92206b0f0b0b7cbc40b1055a81166a8628747c755d5cf61ab0cfb9b61096cf
//META: purpose=A parameter's passing-mode qualifier is part of the parameter's type rather than stored beside it, so the mode appears in the rendered function type of a candidate.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the mode wrapper travels inside the function type: when
// a call fails on argument count, the checker prints the callee's type
// and the printed parameter carries its mode. CHECK pins `func store(out
// int) -> int` and `func accum(inout int) -> int`, which could not be
// recovered if the mode were kept outside the type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int store(out int x) { x = 1; return 0; }
/*CHECK:
    ^^^^^ candidate: func store(out int) -> int
*/
int accum(inout int x) { x = x + 1; return x; }
/*CHECK:
    ^^^^^ candidate: func accum(inout int) -> int
*/

void main()
{
    int r = store();
//CHECK:         ^ not enough arguments to call
//CHECK:         ^ not enough arguments to call (got 0, expected 1)
    int q = accum();
//CHECK:         ^ not enough arguments to call
//CHECK:         ^ not enough arguments to call (got 0, expected 1)
    printf("%d\n", r + q);
}
