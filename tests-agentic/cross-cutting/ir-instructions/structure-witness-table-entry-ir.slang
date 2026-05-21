//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=Boundary probe of the documented `witness_table_entry(requirementKey, satisfyingVal)` opcode: an interface with multiple requirements produces one `witness_table_entry` per requirement in the table.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `witness_table_entry` with operand shape
// `(requirementKey, satisfyingVal)`. The bootstrap covers a single-
// requirement interface; this probe pins down the per-requirement
// axis by giving the interface two requirements (`getX` and `getY`).
// The IR must show two `witness_table_entry` rows in the conforming
// type's table.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IPair { int getX(); int getY(); }
struct A : IPair { int x; int y; int getX() { return x; } int getY() { return y; } }

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    A a;
    a.x = 1;
    a.y = 2;
    buf[0] = a.getX() + a.getY();
}

// IR: witness_table_entry(%{{.*}},%{{.*}})
// IR: witness_table_entry(%{{.*}},%{{.*}})
