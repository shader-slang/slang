//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
//META: purpose=A DeclRefBase references a declaration possibly with generic substitutions; a single generic function declaration can be referenced at two different substitutions and each reference resolves to the appropriate specialization.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that one generic FuncDecl is referenced as two distinct
// DeclRefs — `id<int>` and `id<float>` — and each picks up the
// substitution for its type parameter, producing the appropriate
// return value. If DeclRefBase could not carry substitutions, the
// generic could not be specialized differently at the two call sites
// and the program would not type-check.

//TEST:INTERPRET(filecheck=CHECK):

T id<T>(T v) { return v; }

void main()
{
    int   i = id<int>(7);
    float f = id<float>(2.5f);
    //CHECK: i=7
    printf("i=%d\n", i);
    //CHECK: f=2.500000
    printf("f=%f\n", f);
}
