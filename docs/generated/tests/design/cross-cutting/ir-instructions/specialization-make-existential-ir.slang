//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=96937cffa7eab5a700d8fe30b96ad6032aaa2693bb26bdc6db27157e8ac98d94
//META: purpose=Assigning a concrete type to an interface-typed local lowers to the documented `makeExistential(value, witness)` opcode that packs the value and its witness table.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `makeExistential` with C++ wrapper `MakeExistential`
// and operand shape `(value, witness)`, "Packs a value plus its
// witness". Assigning a concrete `A` (where `A : IFoo`) to a local
// `IFoo` triggers this construction in lowering.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }
struct A : IFoo { int x; int getVal() { return x; } }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 42;
    IFoo iface = a;
    buf[0] = iface.getVal();
}

// IR-LABEL: func %main
// IR: makeExistential(%{{.*}}, %{{.*}})
