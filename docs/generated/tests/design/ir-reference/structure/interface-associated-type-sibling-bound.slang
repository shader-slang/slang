//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e214a09142215b02ab6420756675cda4aea3ee3bccc295e9aa6dbc0ccf6a4a23
//META: purpose=An associated-type bound is a sibling requirement of the associated type itself, carrying a witness-table requirement value, rather than nested entries under it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies how `associatedtype A : IBar` is decomposed. The CHECK-DAGs require
// two peer entries: one keyed on `A` whose requirement value is
// `associated_type`, and one whose requirement value is
// `witness_table_t(%IBar)` — the bound, standing alongside `A` rather than
// inside it. The final CHECK anchors the operand count at three (the two above
// plus the interface's method), so no extra entries were synthesized for the
// bound.

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

// CHECK-DAG: interface_req_entry(%{{[0-9]+}}, witness_table_t(%IBar))
// CHECK-DAG: interface_req_entry(%IHasAssocx5FA, associated_type)
// CHECK: let  %IHasAssoc	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}){{$}}
