//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=An InterfaceDecl lowers to an `IRInterfaceType` plus per-method `interface_req_entry` instructions.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's row "InterfaceDecl → IRInterfaceType plus per-method
// requirement insts". In the dump this appears as an
// `interface_req_entry(<key>, Func(...))` line and an
// `interface(...)` declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int doIt(int x); }
struct A : IFoo { int doIt(int x) { return x + 1; } }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A inst;
    buf[0] = inst.doIt(a);
}

// CHECK: interface_req_entry({{.*}}, Func(Int, this_type({{.*}}IFoo), Int))
// CHECK: let %IFoo : Type = interface
