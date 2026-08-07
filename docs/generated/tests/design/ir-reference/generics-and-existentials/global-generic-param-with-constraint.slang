//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=43b8b0e49573b6e717cbcb06d2dacc7db2424f57bca3c5a939322ea27bf6a4e6
//META: purpose=A constrained module-scope type_param emits a second global_generic_param for the constraint's witness table alongside the one for the type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that the constraint attached to a module-scope generic parameter is
// itself a module-scope generic parameter. `type_param T : IFoo` therefore
// yields two `global_generic_param` values: one typed `Type` for `T` and one
// typed `witness_table_t(%IFoo)` for the conformance, which the in-body
// dispatch reads. The CHECKs pin both, plus the `lookupWitness` that consumes
// the witness parameter.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int run(int x); };

type_param T : IFoo;

uniform T val;
uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = val.run(inVal);
}

// CHECK-DAG: let  %{{[0-9]+}} : Type = global_generic_param
// CHECK-DAG: let  %{{[0-9]+}} : witness_table_t(%IFoo) = global_generic_param
// CHECK-DAG: lookupWitness(%{{[0-9]+}}, %IFoox5Frun)
