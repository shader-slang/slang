//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=561db21a4ad8f3742180c521eb399a01338495f47b8085f196d3a2228c3211cf
//META: purpose=Assigning a concrete conformer to an interface-typed location lowers to a `makeExistential` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `makeExistential`/`MakeExistential` row ("Packs a
// value plus its witness"). Storing a concrete `A` into an `IFoo`-typed
// local packs the value with its witness table, which lowers to
// `makeExistential(%value, %witness)` in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }
struct A : IFoo { int x; int getVal() { return x; } }

int useExistential(IFoo obj) { return obj.getVal(); }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 9;
    buf[0] = useExistential(a);
}

// IR: makeExistential(%{{.*}}, %{{.*}})
