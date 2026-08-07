//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e214a09142215b02ab6420756675cda4aea3ee3bccc295e9aa6dbc0ccf6a4a23
//META: purpose=A property requirement contributes one interface_req_entry per accessor rather than one for the property itself.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-accessor decomposition of a property requirement. The
// interface declares one property with a getter and a setter; the CHECK-DAGs
// require a separate requirement entry keyed on each accessor, and the final
// CHECK anchors the end of the `interface(...)` line to require exactly two
// operands — so a single property-level entry would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IHasProp
{
    property int val { get; set; }
}

struct Cell : IHasProp
{
    int stored;
    property int val { get { return stored; } set { stored = newValue; } }
}

int read<T : IHasProp>(T t) { return t.val; }

[numthreads(1, 1, 1)]
void main()
{
    Cell c;
    c.stored = seed;
    buf[0] = read(c);
}

// CHECK-DAG: interface_req_entry(%IHasPropx5Fvalx5Fget,
// CHECK-DAG: interface_req_entry(%IHasPropx5Fvalx5Fset,
// CHECK: let  %IHasProp	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}}){{$}}
