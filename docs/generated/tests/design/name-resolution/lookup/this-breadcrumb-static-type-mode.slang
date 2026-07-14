//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=b27c30a2892df8994ba4270aff0be7417182c3ff42b3efe8453e905a6ffd8d9f
//META: purpose=A static member function carries a `This` breadcrumb that resolves enclosing static members through the type (not through a value instance), so unqualified `factor` reads the static field.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 5 of the unqualified-lookup walk: `static` changes the
// ThisParameterMode. A static method does not receive a value
// `this`; its breadcrumb refers to the enclosing type itself. The
// unqualified read of `factor` from inside `scale` resolves through
// that type-mode breadcrumb to the static field.

//TEST:INTERPRET(filecheck=CHECK):

struct U
{
    static const int factor = 4;
    static int scale(int x) { return x * factor; }
}

void main()
{
    int r = U::scale(5);
    // CHECK: r=20
    printf("r=%d\n", r);
}
