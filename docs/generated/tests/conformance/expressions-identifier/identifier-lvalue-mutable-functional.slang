//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#identifier-expressions
//META: doc_section_digest=caa6a8edb60e77c5f7baea1fd474acf5d581589b89a50b719ca26405a5bcf66e
//META: purpose=An identifier expression is an l-value when the declaration it refers to is mutable, allowing assignment and inout passing.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When the identifier refers to a mutable local variable, it is an l-value and can be assigned.
// This tests both direct assignment and passing as an `inout` argument.
//TEST:INTERPRET(filecheck=CHECK):

void increment(inout int v) { v = v + 1; }

void main()
{
    int x = 10;
    // 'x' is a mutable local — the identifier expression 'x' is an l-value.
    x = 20;             // direct assignment through identifier l-value
    increment(x);       // passing identifier l-value as inout
    //CHECK: 21
    printf("%d\n", x);
}
