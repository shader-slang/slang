//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=4820552ad726706a9ebecdf715453f85eb93c80c7a2decca221f929866c46c78
//META: purpose=Negative: two identical case labels in a switch are rejected at semantic-check time with E30605 "duplicate cases not allowed" — the IR's `(caseValue, caseBlock)` pair set is required to have unique caseValue entries, so the front-end refuses to lower a malformed switch.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `case 1:` arms in the same switch trigger E30605 at semantic
// check time; the IR-level switch terminator's case-value operand
// list would otherwise have to host a duplicate, so the malformed
// surface form is rejected before lowering.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (sel)
    {
    case 1:
        r = 10;
        break;
    case 1:
/*CHECK:
    ^^^^ duplicate cases in switch
*/
// CHECK: E30605
        r = 20;
        break;
    default:
        r = 0;
        break;
    }
    buf[0] = r;
}
