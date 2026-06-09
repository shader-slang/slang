//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
//META: purpose=A WitnessTable synthesized in checking lowers to an `IRWitnessTable` whose entries map interface requirements to concrete implementations.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "Witness tables (computed by slang-check-conformance)
// become IRWitnessTable insts whose entries map interface
// requirements to the concrete implementations." In the IR dump we
// see a `witness_table %{{.*}} : witness_table_t(%IFoo)(%A)`
// declaration with a `witness_table_entry(%req, %impl)` row.

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

// CHECK: witness_table %{{.*}} : witness_table_t(%IFoo)(%A)
// CHECK: witness_table_entry(%{{.*}},%{{.*}})
