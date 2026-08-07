//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=An InterfaceDecl with exactly one method lowers to one `interface_req_entry(...)` row plus an `interface(...)` declaration with that single requirement.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: interface-method-count axis at exactly one — the
// minimum non-empty case. Sibling of the zero (`interfacedecl-empty`)
// and three / eight cases. The doc claim "InterfaceDecl ->
// IRInterfaceType plus per-method requirement insts" must hold here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IOne { int act(int v); }
struct A : IOne { int act(int v) { return v + 1; } }

uniform int x;
uniform RWStructuredBuffer<int> buf;

int use<T : IOne>(T t, int w) { return t.act(w); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A inst;
    buf[0] = use<A>(inst, x);
}

// CHECK: interface_req_entry(%IOnex5Fact
// CHECK: let  %IOne	: Type	= interface(
