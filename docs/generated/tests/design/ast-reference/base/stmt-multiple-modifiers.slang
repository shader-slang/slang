//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#stmt-modifiablesyntaxnode
//META: doc_section_digest=8ea5e05b7dcdf6004362460ead8f7e0c770d0d1fd47a885f5710ce2e461e0a0f
//META: purpose=The modifiers list a Stmt inherits is a list, so several attributes may be stacked in front of one statement and all of them attach.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a statement's modifier storage holds more than one
// entry. Two attributes are stacked on a single `for`; if only one
// slot existed the second would be dropped or diagnosed. CHECK asserts
// the loop still runs to completion with both attached, and the
// `[MaxIters(8)]` bound is satisfied by the five iterations performed.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    [loop]
    [MaxIters(8)]
    for (int i = 0; i < 5; ++i)
    {
        sum = sum + i;
    }
    //CHECK: sum=10
    printf("sum=%d\n", sum);
}
