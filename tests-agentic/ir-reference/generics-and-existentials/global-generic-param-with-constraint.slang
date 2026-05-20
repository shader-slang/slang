//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=8f96c600fb4e9a50947a60ac22318e6fba6c6386abe0fabef983a2b0e0a9b785
//META: purpose=Two distinct type_param declarations at module scope each lower to a global_generic_param — pluralised global generic params are independent IR values.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on global_generic_param multiplicity: two separate
// module-scope `type_param` declarations each produce their own
// `let %X : Type = global_generic_param` line. The doc's
// `### Generic application` row treats each
// `GenericTypeParamDecl (at module scope)` as a single
// declaration. The original "with-constraint" variant probe
// (constrained `type_param T : IFoo`) crashes the compiler
// downstream; that crash is recorded as a doc-gap in BUNDLE.md.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

type_param T;
type_param U;

uniform RWStructuredBuffer<T> bufT;
uniform RWStructuredBuffer<U> bufU;
uniform T valT;
uniform U valU;

[numthreads(1, 1, 1)]
void main()
{
    bufT[0] = valT;
    bufU[0] = valU;
}

// CHECK: let  %{{[0-9]+}}	: Type	= global_generic_param
// CHECK: let  %{{[0-9]+}}	: Type	= global_generic_param
