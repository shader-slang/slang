//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#astypeexpr--istypeexpr--casttosupertypeexpr
//META: doc_section_digest=a1b5995779c673c26a303bd7b4c37473fda1915905c75120601e573453457f3a
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
