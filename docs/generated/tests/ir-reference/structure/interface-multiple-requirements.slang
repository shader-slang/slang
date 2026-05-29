//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#interface-internals
//META: doc_section_digest=881ba3684ff6d20a2e1a262e82de634f0b857cc0f62fb483fbb5984cc6845bdf
//META: purpose=An interface with multiple method requirements emits one interface_req_entry per requirement, and the interface opcode lists all of them as its operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `interface ITwo { int up(int x); int dn(int x); }` should produce
// two `interface_req_entry` rows — one per method — and the
// `interface(...)` opcode lists both of them as operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface ITwo
{
    int up(int x);
    int dn(int x);
};

struct C : ITwo
{
    int up(int x) { return x + 1; }
    int dn(int x) { return x - 1; }
};

uniform int v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    C c;
    buf[0] = c.up(v) + c.dn(v);
}

// CHECK: interface_req_entry(%ITwox5Fup,
// CHECK: interface_req_entry(%ITwox5Fdn,
// CHECK: let  %ITwo	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}})
