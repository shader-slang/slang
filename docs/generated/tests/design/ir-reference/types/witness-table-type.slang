//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#witness-table-types
//META: doc_section_digest=bfbd3530f0dbe36c90209d99b70c7379641a73a0001f242585f5aaef81b2437f
//META: purpose=A conformance produces a witness_table value whose IR type is witness_table_t(<interface>) with witness_table_entry rows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A conformance produces a witness_table value whose IR type is
// witness_table_t(<interface>) with witness_table_entry rows. The
// conformance of `Square : IShape` emits `witness_table %... :
// witness_table_t(%IShape)(%Square)` with one `witness_table_entry`
// per requirement.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { float area(); }
struct Square : IShape { float side; float area() { return side * side; } }

uniform float s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    Square sq = { s };
    buf[0] = sq.area();
}

// IR: witness_table %{{.*}}: witness_table_t(%IShape)(%Square)
// IR: witness_table_entry(%{{.*}}
