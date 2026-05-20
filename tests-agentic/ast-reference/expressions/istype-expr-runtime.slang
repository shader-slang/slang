//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr
//META: doc_section_digest=8acefa9f31e1283f2e07f4a5c07a19625522aed173eb000464e0d7fc4693e965
//META: purpose=An IsTypeExpr `value is Type` is a runtime type test: it yields true when the dynamic type matches.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// IsTypeExpr is documented as `value is Type` with a compile-time
// witnessArg. We exercise it through an interface variable whose
// dynamic type we know; the `is` check must select the right arm.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int area(); }
struct Sq : IShape { int s; int area() { return s * s; } }
struct Tr : IShape { int b; int h; int area() { return b * h / 2; } }

int classify(IShape shape)
{
    if (shape is Sq) return 1;
    if (shape is Tr) return 2;
    return 0;
}

void main()
{
    Sq sq = {3};
    Tr tr = {4, 5};
    // CHECK: sq=1
    printf("sq=%d\n", classify(sq));
    // CHECK: tr=2
    printf("tr=%d\n", classify(tr));
}
