//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#astypeexpr--istypeexpr--casttosupertypeexpr
//META: doc_section_digest=a1b5995779c673c26a303bd7b4c37473fda1915905c75120601e573453457f3a
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
