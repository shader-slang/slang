//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=96937cffa7eab5a700d8fe30b96ad6032aaa2693bb26bdc6db27157e8ac98d94
//META: purpose=Calling an interface-constrained generic on a concrete type lowers to the documented `lookupWitness(witnessTable, requirementKey)` opcode that resolves the interface requirement to the concrete method.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `lookupWitness` with C++ wrapper `LookupWitnessMethod`
// and operand shape `(witnessTable, requirementKey)`. A generic
// constrained by `T:IFoo` calling `item.getVal()` produces this
// opcode: the witness table for `T:IFoo` is looked up against the
// `getVal` requirement key, yielding the concrete method to call.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }
struct A : IFoo { int x; int getVal() { return x; } }

uniform RWStructuredBuffer<int> buf;

int callIt<T:IFoo>(T item) { return item.getVal(); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 42;
    buf[0] = callIt(a);
}

// IR: lookupWitness({{.*}}, %IFoox5FgetVal)
