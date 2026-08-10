//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e214a09142215b02ab6420756675cda4aea3ee3bccc295e9aa6dbc0ccf6a4a23
//META: purpose=A default implementation written in an interface body adds no requirement entry of its own, so the interface keeps one entry per declared requirement.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the synthesized decl holding a default body is skipped when the
// requirement list is built. The interface declares two methods, one of which
// carries a default body; CHECK anchors the end of the `interface(...)` line to
// require exactly two operands. A third entry would mean the default-impl decl
// had also been treated as a requirement.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IWithDefault
{
    int base(int x);
    int withDefault(int x) { return base(x) + 1; }
}

struct S : IWithDefault { int base(int x) { return x * 2; } }

int use<T : IWithDefault>(T t, int x) { return t.withDefault(x); }

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = use(s, seed);
}

// CHECK-DAG: interface_req_entry(%IWithDefaultx5Fbase,
// CHECK-DAG: interface_req_entry(%IWithDefaultx5FwithDefault,
// CHECK: let  %IWithDefault	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}}){{$}}
