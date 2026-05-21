//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=An InterfaceDecl with five methods lowers to five `interface_req_entry(...)` rows referenced by a single `interface(%1, %2, %3, %4, %5)` declaration, scaling the "InterfaceDecl -> IRInterfaceType plus per-method requirement insts" claim.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: interface-method-count "many" axis. Sibling of the
// 1 / 3-method tests; verifies that the per-method
// `interface_req_entry` row scales to five distinct methods, each
// preserving the requirement name in source order.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFive {
    int a(int x);
    int b(int x);
    int c(int x);
    int d(int x);
    int e(int x);
}

struct A : IFive {
    int a(int x) { return x; }
    int b(int x) { return x + 1; }
    int c(int x) { return x + 2; }
    int d(int x) { return x + 3; }
    int e(int x) { return x + 4; }
}

uniform int v;
uniform RWStructuredBuffer<int> buf;

int use<T : IFive>(T t, int w) { return t.a(w) + t.b(w) + t.c(w) + t.d(w) + t.e(w); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A inst;
    buf[0] = use<A>(inst, v);
}

// CHECK: interface_req_entry(%IFivex5Fa
// CHECK: interface_req_entry(%IFivex5Fb
// CHECK: interface_req_entry(%IFivex5Fc
// CHECK: interface_req_entry(%IFivex5Fd
// CHECK: interface_req_entry(%IFivex5Fe
