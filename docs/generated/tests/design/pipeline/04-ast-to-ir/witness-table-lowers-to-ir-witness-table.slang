//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d11d731c1f470905e85d4dccd140c912ec50e6d88afbda6ff67159fa80cd78b2
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
