//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=821d70752ab2fb0ab927aca034dc6d1ab1bf2642184d0e87594d63c182097322
//META: purpose=A GenericDecl wraps an inner decl with a parameter list; the same inner decl specializes differently at distinct type arguments.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of GenericDecl: one generic
// FunctionDecl serves as two distinct specializations (`int` and
// `float`) at separate call sites, with the parameter list at the
// generic wrapper feeding the inner decl.

//TEST:INTERPRET(filecheck=CHECK):

T pickFirst<T>(T a, T b) { return a; }

void main()
{
    //CHECK: 7 2.500000
    printf("%d %f\n", pickFirst<int>(7, 8), pickFirst<float>(2.5f, 9.0f));
}
