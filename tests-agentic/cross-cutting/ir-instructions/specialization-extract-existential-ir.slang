//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=96937cffa7eab5a700d8fe30b96ad6032aaa2693bb26bdc6db27157e8ac98d94
//META: purpose=Calling an interface method through an existential-typed value projects the value, type, and witness via the documented `extractExistentialValue` / `extractExistentialType` / `extractExistentialWitnessTable` opcodes.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `extractExistentialValue`, `extractExistentialType`,
// and `extractExistentialWitnessTable` as "Existential projections"
// taking a single `existential` operand. All three appear when a
// method call goes through an interface-typed local.

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
// IR: extractExistentialType(%iface)
// IR: extractExistentialValue(%iface)
// IR: extractExistentialWitnessTable(%iface)
