//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=6c19be2b98b712dfb44e27eac1a62628a55a8795cf5a111496e7292369d1578c
//META: purpose=A two-method interface conformance lowers to an `IRWitnessTable` carrying two `witness_table_entry(req, impl)` rows — one per interface requirement.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: witness-table entry-count axis. The doc says
// "IRWitnessTable insts whose entries map interface requirements to
// the concrete implementations" — at two methods, the table carries
// two entries, naming each requirement and each implementation.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { int area(); int perim(); }
struct Sq : IShape {
    int side;
    int area() { return side * side; }
    int perim() { return side * 4; }
}

uniform int s;
uniform RWStructuredBuffer<int> buf;

int total<T : IShape>(T sh) { return sh.area() + sh.perim(); }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Sq sq;
    sq.side = s;
    buf[0] = total<Sq>(sq);
}

// CHECK: witness_table %{{.*}} : witness_table_t(%IShape)(%Sq)
// CHECK: witness_table_entry(%IShapex5Farea
// CHECK: witness_table_entry(%IShapex5Fperim
