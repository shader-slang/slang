//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=6c67481fccb6a5d96551c93af7509ffafa480d11451126832bec4a0352c4b938
//META: purpose=Operator overloading: a user-defined `operator+` on a struct is found by overload resolution at a `+` expression with that struct's argument types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The overload-resolution doc describes operator overloading as a
// special candidate-set query keyed by the operator. Verify that
// a user-defined `operator+` on a struct is selected.

//TEST:INTERPRET(filecheck=CHECK):

struct V { int a; int b; }

V operator+(V x, V y)
{
    V r;
    r.a = x.a + y.a;
    r.b = x.b + y.b;
    return r;
}

void main()
{
    V p = { 1, 2 };
    V q = { 10, 20 };
    V s = p + q;
    // CHECK: s=11,22
    printf("s=%d,%d\n", s.a, s.b);
}
