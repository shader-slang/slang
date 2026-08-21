//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=100b91a908217e68795cbc9dac7393e0325718d4c3df831fe3616ee38716a45a
//META: purpose=A module-level type_param declaration lowers to global_generic_param, and its interface constraint lowers to a second global_generic_param holding the witness table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the module-level generic-parameter surface produces
// `global_generic_param` and that a constraint decl parented by it lowers to
// the same opcode. The CHECK-DAGs require two module-scope insts: one typed
// `Type` (the type parameter `T` itself) and one typed
// `witness_table_t(%IFoo)` (the `T : IFoo` constraint), which is how a body can
// dispatch `t.run(x)` before `T` is bound.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IFoo { int run(int x); }

type_param T : IFoo;

int callRun(T t, int x) { return t.run(x); }

[numthreads(1, 1, 1)]
void main()
{
    T t = {};
    buf[0] = callRun(t, seed);
}

// CHECK-DAG: let  %{{[0-9]+}}	: Type	= global_generic_param
// CHECK-DAG: let  %{{[0-9]+}}	: witness_table_t(%IFoo)	= global_generic_param
