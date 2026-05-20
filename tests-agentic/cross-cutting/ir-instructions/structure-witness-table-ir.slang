//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=A type that conforms to an interface produces a `witness_table` with `witness_table_entry` children mapping interface requirements to satisfying values.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `struct A : IFoo { ... }` conformance produces a witness table
// (the doc's "Witness table machinery" entry). The table is a top-
// level `witness_table` whose children are `witness_table_entry`
// rows mapping each `interface_req_entry` key to the satisfying
// method.

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
