//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=Each BasicExpressionType (int, uint, float, bool) behaves as the corresponding scalar built-in; its value flows through evaluation as that type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies BasicExpressionType is the arithmetic-type leaf for the
// scalar built-ins: each surface spelling produces a value typed as
// the corresponding scalar. Several scalars are grouped in one test;
// CHECK asserts each round-trips at its own type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = -5;
    uint u = 7u;
    float f = 1.5f;
    bool b = true;

    // CHECK: i=-5
    printf("i=%d\n", i);
    // CHECK: u=7
    printf("u=%d\n", int(u));
    // CHECK: f=1
    printf("f=%d\n", int(f));
    // CHECK: b=1
    printf("b=%d\n", b ? 1 : 0);
}
