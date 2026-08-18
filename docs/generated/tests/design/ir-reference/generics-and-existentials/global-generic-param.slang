//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=e7e100a63c866d0df80fc71ebcb3d885933673afb6bac3a23c94ac816f7b50b1
//META: purpose=A module-scope type_param T declaration lowers to let %T : Type = global_generic_param.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that a generic parameter declared at module scope, rather than on a
// declaration, becomes its own module-scope IR value. The buffer element type
// is `T`, so the parameter cannot be folded away, and the CHECK pins the
// `global_generic_param` line with its `Type` result.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

type_param T;

uniform RWStructuredBuffer<T> buf;
uniform T val;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = val;
}

// CHECK: let  %{{[0-9]+}} : Type = global_generic_param
