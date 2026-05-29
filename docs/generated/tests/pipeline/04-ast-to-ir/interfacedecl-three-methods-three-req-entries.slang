//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=An InterfaceDecl with three methods lowers to one `interface_req_entry(...)` instruction per method; the interface type aggregates them into a single `interface(...)` operand list.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: interface-method-count "many" axis. Three distinct
// methods produce three `interface_req_entry` rows referenced from
// the `interface(%1, %2, %3)` declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IThree {
    int a(int x);
    int b(int x);
    int c(int x);
}

struct A : IThree {
    int a(int x) { return x; }
    int b(int x) { return x * 2; }
    int c(int x) { return x + 1; }
}

uniform int v;
uniform RWStructuredBuffer<int> buf;

int use<T : IThree>(T t, int w) { return t.a(w) + t.b(w) + t.c(w); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A inst;
    buf[0] = use<A>(inst, v);
}

// CHECK: interface_req_entry(%IThreex5Fa
// CHECK: interface_req_entry(%IThreex5Fb
// CHECK: interface_req_entry(%IThreex5Fc
// CHECK: let  %IThree	: Type	= interface(
