//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#astypeexpr--istypeexpr--casttosupertypeexpr
//META: doc_section_digest=a1b5995779c673c26a303bd7b4c37473fda1915905c75120601e573453457f3a
//META: purpose=Verifies the false branch of IsTypeExpr: `value is OtherImpl` yields false when the dynamic type does not match.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative-side boundary for IsTypeExpr.constantVal: when the
// witness check rejects, the expression evaluates to false.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int kind(); }

struct Circle : IShape { int kind() { return 1; } }
struct Square : IShape { int kind() { return 2; } }

void main()
{
    IShape s = Circle();
    // CHECK: isSquare=0
    printf("isSquare=%d\n", int(s is Square));
}
