//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#symbolalias
//META: doc_section_digest=df33340c4bf4fae395203cc1b56c122f7e9cf3491ebdffc3e445b13502d15254
//META: purpose=A link-time export type alias is created during lowering as a SymbolAlias whose operand is the aliased symbol, and the conformance nested in it gets its own SymbolAlias.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `SymbolAlias` is produced by lowering rather than by the
// linker, and that both halves of an aliased conforming type are aliased. The
// CHECK-DAGs require one alias whose result type is `Type` and whose operand is
// the concrete `%FooImpl`, plus a second alias typed as a witness table for the
// same interface — the conformance standing in for the alias's own.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IFoo { int run(int x); }

export struct FooImpl : IFoo { int run(int x) { return x + 1; } }

export struct Foo : IFoo = FooImpl;

[numthreads(1, 1, 1)]
void main()
{
    FooImpl f;
    buf[0] = f.run(inVal);
}

// CHECK-DAG: let  %{{[0-9]+}}	: Type	= SymbolAlias(%FooImpl)
// CHECK-DAG: let  %{{[0-9]+}}	: witness_table_t(%IFoo)	= SymbolAlias(%{{[0-9]+}})
