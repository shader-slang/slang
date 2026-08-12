//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=44c1034c52fc65241dd638ce5a86a08822f7e7eef48bfb8b6c16e47820038dcf
//META: purpose=Stresses LambdaExpr nesting: a curried `(int x) => (int y) => x + y` parses, captures `x` into the inner lambda, and yields the documented sum.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two LambdaExpr nodes nested. The inner lambda captures the outer
// lambda's parameter `x`. Currying `add(5)(7)` must yield 12.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var add = (int x) => (int y) => x + y;
    var addToFive = add(5);
    // CHECK: r=12
    printf("r=%d\n", addToFive(7));
}
