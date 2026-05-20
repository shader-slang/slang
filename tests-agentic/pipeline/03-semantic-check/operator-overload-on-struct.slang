//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#operator-overloading
//META: doc_section_digest=017e2a9ca7d5b435c5665a1115eba51fbc2b30352078a8ab4c0e65601bfa2273
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
