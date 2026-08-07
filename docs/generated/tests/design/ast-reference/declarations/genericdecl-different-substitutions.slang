//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=72472a48bac619938006bb23a0e1ce51075ed1e262b7eb32a455b8764ff2fc94
//META: purpose=A single GenericDecl can be specialized with different value/type substitutions per call site; each specialization is independent.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of GenericDecl as the wrapper that
// "lets the rest of the front end treat a generic as 'the inner decl
// with a parameter list'". One generic struct declaration is used at
// two distinct type substitutions and each instantiation carries its
// own storage type.

//TEST:INTERPRET(filecheck=CHECK):

struct Holder<T>
{
    T x;
    T get() { return x; }
}

void main()
{
    Holder<int>   hi; hi.x = 100;
    Holder<float> hf; hf.x = 1.5f;
    //CHECK: 100 1.500000
    printf("%d %f\n", hi.get(), hf.get());
}
