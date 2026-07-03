//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=72472a48bac619938006bb23a0e1ce51075ed1e262b7eb32a455b8764ff2fc94
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
