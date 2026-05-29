//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=8f96c600fb4e9a50947a60ac22318e6fba6c6386abe0fabef983a2b0e0a9b785
//META: purpose=A module-scope type_param T declaration lowers to let %T : Type = global_generic_param.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### Generic application` lists `global_generic_param` with AST
// origin "`GenericTypeParamDecl` (at module scope)". The Slang
// surface for that is the `type_param` keyword at module scope.
// It lowers to a top-level `let %T : Type = global_generic_param`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

type_param T;

uniform RWStructuredBuffer<T> buf;
uniform T val;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = val;
}

// CHECK: let  %{{[0-9]+}}	: Type	= global_generic_param
