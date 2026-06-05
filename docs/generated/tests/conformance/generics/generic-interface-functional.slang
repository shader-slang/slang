//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic interface with type parameter can be implemented by a struct, and a generic function constrained to that interface can be called with the implementing struct.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's grammar shows generic interface syntax: `interface Identifier<generic-params-decl>`.
// A generic interface parameterizes the interface itself. Here IContainer<T> provides a
// `get` method returning T.

//TEST:INTERPRET(filecheck=CHECK):

interface IContainer<T>
{
    T get();
}

struct IntBox : IContainer<int>
{
    int _val;
    int get() { return _val; }
}

struct FloatBox : IContainer<float>
{
    float _val;
    float get() { return _val; }
}

void main()
{
    IntBox ib;
    ib._val = 77;
    FloatBox fb;
    fb._val = 3.0;
    // CHECK: 77
    printf("%d\n", ib.get());
    // CHECK: 3
    printf("%d\n", (int)fb.get());
}
