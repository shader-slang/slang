//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witness-tables-and-witness-facts
//META: doc_section_digest=1af9e6c2e01af2b94483cbd8d90b8d30f4856731a44da18cf2e68c7379930ffc
//META: purpose=A where-clause equating an associated type with a concrete type lowers to a TypeEqualityWitness carrying the sub type and super type as operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the AST-level type-equality witness has a direct IR
// counterpart with a two-operand shape. The `where T.A == BarImpl` constraint
// is the surface that produces it; CHECK requires a
// `TypeEqualityWitness(<subType>, <superType>)` inst whose result type is a
// witness-table type, which is what the generic body then uses to treat
// `t.make(x)` as a `BarImpl`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IBar { int bar(); }

interface IHasAssoc
{
    associatedtype A : IBar;
    A make(int x);
}

struct BarImpl : IBar { int v; int bar() { return v; } }

struct Holder : IHasAssoc
{
    typedef BarImpl A;
    A make(int x) { BarImpl b; b.v = x; return b; }
}

int useEq<T : IHasAssoc>(T t, int x) where T.A == BarImpl
{
    BarImpl b = t.make(x);
    return b.bar();
}

[numthreads(1, 1, 1)]
void main()
{
    Holder h;
    buf[0] = useEq(h, seed);
}

// CHECK: let  %{{[0-9]+}}	: witness_table_t(%BarImpl)	= TypeEqualityWitness(%BarImpl, %BarImpl)
