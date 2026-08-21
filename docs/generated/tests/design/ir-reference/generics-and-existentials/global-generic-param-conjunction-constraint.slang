//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=e7e100a63c866d0df80fc71ebcb3d885933673afb6bac3a23c94ac816f7b50b1
//META: purpose=A conjunction constraint on a module-scope type_param emits one witness global_generic_param per half, so `T : IFoo & IBar` lowers to three module-scope parameters.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that the "one witness parameter per constraint" rule counts each
// half of a conjunction separately rather than producing a single parameter for
// the conjunction as a whole. `type_param T : IFoo & IBar` is declared with two
// interfaces; the CHECK-DAGs pin the `Type`-typed parameter for `T` plus one
// `witness_table_t` parameter naming `%IFoo` and another naming `%IBar`. A
// single fused witness parameter would leave one of the last two unmatched.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IFoo { int foo(int x); };
interface IBar { int bar(int x); };

type_param T : IFoo & IBar;

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 0;
}

// CHECK-DAG: let  %{{[0-9]+}} : Type = global_generic_param
// CHECK-DAG: let  %{{[0-9]+}} : witness_table_t(%IFoo) = global_generic_param
// CHECK-DAG: let  %{{[0-9]+}} : witness_table_t(%IBar) = global_generic_param
