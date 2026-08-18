//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=d6d21d15e8b14ba2b57495447b3188f156d4c4168b8574097962151b591b1497
//META: purpose=A type that conforms to an interface produces a `witness_table` with `witness_table_entry` children mapping requirements to satisfying values.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `witness_table`/`witness_table_entry` structure row
// ("Witness table machinery") and the `interface`/`InterfaceType` type
// row. A `struct A : IFoo` conformance emits a top-level `witness_table`
// whose `witness_table_entry` rows map each requirement key to its
// satisfying method.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }
struct A : IFoo { int x; int getVal() { return x; } }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 42;
    buf[0] = a.getVal();
}

// IR-LABEL: let %IFoo : Type = interface
// IR: witness_table %{{.*}} : witness_table_t(%IFoo)(%A)
// IR: witness_table_entry(%{{.*}},%{{.*}})
