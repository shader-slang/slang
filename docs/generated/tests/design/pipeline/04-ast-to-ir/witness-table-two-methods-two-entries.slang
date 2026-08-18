//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=f9fbc5ab50420dd2b267ddb86a5eff193a88f8bd53dbaeb86a9ca9fdcaf05b23
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
