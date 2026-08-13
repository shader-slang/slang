//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=A ClassDecl declares a reference-type aggregate constructed with `new`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a class is the reference-type aggregate variant, allocated
// with `new` rather than copied like a struct. The interpreter cannot
// instantiate classes, so the observation point is the emitted C++ text; the
// CHECK pins the `new C` allocation, which only appears for a reference type.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

class C
{
    int v;
    __init(int x) { v = x; }
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    //CHECK: new C
    C a = new C(7);
}
