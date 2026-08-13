//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=A property requirement contributes one interface_req_entry per accessor rather than one for the property itself.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Boundary on what counts as a requirement. `IHasProp` declares two members —
// a property and a method — but the property is not itself a requirement; its
// getter and setter are. The CHECK-DAGs name the two accessor-derived keys and
// the method key, and the last one pins the interface's operand list at exactly
// three entries, so an extra entry for the property itself would fail.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IHasProp
{
    property int val { get; set; }
    int plain(int x);
};

struct A : IHasProp
{
    int stored;
    property int val { get { return stored; } set { stored = newValue; } }
    int plain(int x) { return x + 1; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int use<T : IHasProp>(T t, int x) { return t.val + t.plain(x); }

[numthreads(1, 1, 1)]
void main()
{
    A v;
    v.stored = inVal;
    buf[0] = use<A>(v, inVal);
}

// CHECK-DAG: interface_req_entry(%IHasPropx5Fvalx5Fget,
// CHECK-DAG: interface_req_entry(%IHasPropx5Fvalx5Fset,
// CHECK-DAG: interface_req_entry(%IHasPropx5Fplain,
// CHECK-DAG: let  %IHasProp : Type = interface(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
