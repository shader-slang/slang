//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=An InterfaceDecl lowers to an `IRInterfaceType` whose per-method `interface_req_entry` rows are keyed by requirement-key insts.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's row: an InterfaceDecl becomes an IRInterfaceType whose
// requirement entries are keyed by requirement keys (ordinary
// methods get per-decl IRStructKeys). In the dump this appears as an
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
