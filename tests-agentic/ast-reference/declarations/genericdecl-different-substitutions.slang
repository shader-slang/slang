//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=821d70752ab2fb0ab927aca034dc6d1ab1bf2642184d0e87594d63c182097322
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
