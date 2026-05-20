//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#declrefbase-val
//META: doc_section_digest=d2291ebf5df30d73489e8195c80420773e2998c18564f925e18ba7477ef08a77
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
