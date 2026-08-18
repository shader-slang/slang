//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
