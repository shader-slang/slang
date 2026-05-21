//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
