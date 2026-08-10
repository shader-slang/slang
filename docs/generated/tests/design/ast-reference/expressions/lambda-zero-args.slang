//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=44c1034c52fc65241dd638ce5a86a08822f7e7eef48bfb8b6c16e47820038dcf
//META: purpose=Verifies that a LambdaExpr with zero parameters — `() => 42` — forms a callable that returns the body's constant.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary on LambdaExpr parameter count. The lambda has
// no parameters; invocation must still return the body's value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var get42 = () => 42;
    // CHECK: r=42
    printf("r=%d\n", get42());
}
