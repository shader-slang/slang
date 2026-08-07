//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witness-tables-and-witness-facts
//META: doc_section_digest=1af9e6c2e01af2b94483cbd8d90b8d30f4856731a44da18cf2e68c7379930ffc
//META: purpose=Inside an interface definition the placeholder witness that ThisType conforms to the enclosing interface is a zero-operand thisTypeWitness whose interface is carried in the result type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand-vs-result-type split on `thisTypeWitness`. An interface
// whose method returns its own associated type has to look that type up on
// `This` while the interface is still being defined, which is what materializes
// the placeholder. CHECK anchors the end of the line to require the inst to
// have no operand list at all, while its result type names the enclosing
// interface.

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

int use<T : IHasAssoc>(T t, int x) { return t.make(x).bar(); }

[numthreads(1, 1, 1)]
void main()
{
    Holder h;
    buf[0] = use(h, seed);
}

// CHECK: let  %{{[0-9]+}}	: witness_table_t(%IHasAssoc)	= thisTypeWitness{{$}}
