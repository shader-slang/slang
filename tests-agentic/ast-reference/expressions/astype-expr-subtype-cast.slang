//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#astypeexpr-istypeexpr-casttosupertypeexpr
//META: doc_section_digest=8acefa9f31e1283f2e07f4a5c07a19625522aed173eb000464e0d7fc4693e965
//META: purpose=An AsTypeExpr `value as Type` yields an Optional<Type> populated when the dynamic type matches; the value is recoverable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AsTypeExpr is documented as a subtype cast carrying a witnessArg.
// In Slang, `value as Type` yields `Optional<Type>`; calling
// `.value` on the Optional after `.hasValue` reads the recovered
// concrete value.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int area(); }
struct Sq : IShape { int s; int area() { return s * s; } }

int describe(IShape shape)
{
    if (shape is Sq)
    {
        Optional<Sq> opt = shape as Sq;
        if (opt.hasValue) return opt.value.area();
    }
    return -1;
}

void main()
{
    Sq sq = {4};
    // CHECK: a=16
    printf("a=%d\n", describe(sq));
}
