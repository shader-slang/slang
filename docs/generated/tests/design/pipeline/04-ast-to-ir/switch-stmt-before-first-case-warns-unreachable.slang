//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#diagnostics-during-lowering
//META: doc_section_digest=648027637f6fa9553cdccbac6872d44ba7b96fd6439288fc7af0d43ddadf8360
//META: purpose=A statement placed inside a `switch` body before the first `case` label is unreachable and lowering warns once for that leading run.
//META: intent=negative
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lowering-time unreachability report: control can only enter a
// switch body through the dispatch to a case/default label, so a statement
// written before the first label can never execute. The assignment `r = 7;`
// sits ahead of `case 0:`; the CHECK pins warning code E41000 at the caret
// column of that statement, which is the only place the diagnostic can come
// from (semantic checking itself accepts the statement, and the emitted code
// simply drops it).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    if (a > 0)
    {
        switch (a)
        {
            r = 7;
//CHECK:    ^ E41000
        case 0:
            r = 1;
            break;
        default:
            r = 2;
            break;
        }
    }
    buf[0] = r;
}
