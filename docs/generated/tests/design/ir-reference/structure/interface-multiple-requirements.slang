//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#interface-internals
//META: doc_section_digest=ea89e6f41cb0b27896567604f40c35e8dfe6d5bce27e8dd06de9c29f73a18b9b
//META: purpose=An interface with two method requirements emits one interface_req_entry per requirement and lists both as operands of the interface opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercises the requirement-count axis and the operand-count claim that
// `getRequirementCount()` reads back. Two methods are declared; the ordered
// CHECKs require an entry per method, and the final CHECK requires the
// `interface(...)` operand list to hold exactly two values by anchoring the end
// of the line, so a third synthesized entry would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface ITwo
{
    int up(int x);
    int dn(int x);
}

struct C : ITwo
{
    int up(int x) { return x + 1; }
    int dn(int x) { return x - 1; }
}

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
// CHECK: let  %ITwo	: Type	= interface(%{{[0-9]+}}, %{{[0-9]+}}){{$}}
