//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#stmt-modifiablesyntaxnode
//META: doc_section_digest=8ea5e05b7dcdf6004362460ead8f7e0c770d0d1fd47a885f5710ce2e461e0a0f
//META: purpose=A Stmt inherits a modifiers list, which is why an attribute may be written in front of a statement and still leave the statement type-checking and executing.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an attribute written before a statement attaches to
// that statement instead of being rejected. `[unroll]` sits in front
// of a `for`; CHECK asserts the loop still runs and accumulates
// 0+1+2+3, which requires the annotated statement to have checked and
// executed normally.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        sum = sum + i;
    }
    //CHECK: sum=6
    printf("sum=%d\n", sum);
}
