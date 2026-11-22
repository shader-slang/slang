//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=b99cd634d7767c3e608ac1457ec02c747a3002b11e0c6ec01b999815f7b996c1
//META: purpose=A module-level type_param declaration lowers to global_generic_param, and its interface constraint lowers to a second global_generic_param holding the witness table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the module-level generic-parameter surface produces
// `global_generic_param` and that a constraint decl parented by it lowers to
// the same opcode. The CHECK-DAGs require two module-scope insts: one typed
// `Type` (the type parameter `T` itself) and one typed
// `witness_table_t(%IFoo)` (the `T : IFoo` constraint), the witness inst that
// lets a body call an interface method on `T` before `T` is bound. `T` is
// deliberately not used in shader code: the compiler rejects that (E38207)
// because a global generic parameter needs a concrete binding to be used.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IFoo { int run(int x); }

type_param T : IFoo;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = seed;
}

// CHECK-DAG: let  %{{[0-9]+}}	: Type	= global_generic_param
// CHECK-DAG: let  %{{[0-9]+}}	: witness_table_t(%IFoo)	= global_generic_param
