//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#genericdecl
//META: doc_section_digest=a2a3ea7028c33b102ee0481668bced4a8344b439c1722b28c04420d18b915e69
//META: purpose=A single GenericDecl can be specialized with different substitutions per use site and each specialization keeps its own storage type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two specializations of one generic struct are independent
// types rather than a shared one. Values of the `int` and `float`
// instantiations are stored and read back through the generic's method; the
// CHECK asserts both, so a shared storage type would lose one of them.

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
