//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=561db21a4ad8f3742180c521eb399a01338495f47b8085f196d3a2228c3211cf
//META: purpose=Calling an interface method inside a generic lowers to a `lookupWitness` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `lookupWitness`/`LookupWitnessMethod` row ("Resolves
// an interface requirement"). Inside the generic `callVal<T : IFoo>`,
// calling `x.getVal()` resolves the requirement through the witness
// table, which lowers to `lookupWitness(%witnessTable, %key)` in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }
struct A : IFoo { int x; int getVal() { return x; } }

int callVal<T : IFoo>(T x) { return x.getVal(); }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 5;
    buf[0] = callVal(a);
}

// IR: lookupWitness(%{{.*}}, %{{.*}})
