//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr
//META: doc_section_digest=8acefa9f31e1283f2e07f4a5c07a19625522aed173eb000464e0d7fc4693e965
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
