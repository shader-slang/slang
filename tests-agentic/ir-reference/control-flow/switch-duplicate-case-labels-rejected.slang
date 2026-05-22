//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=cc1b75a0316908fad8da474d769a60fbe3fbf9298cbe56506c4cd716ae3d4b7f
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
