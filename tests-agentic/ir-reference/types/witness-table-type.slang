//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#witness-table-types
//META: doc_section_digest=06b14fdf48b80a42b7d80961315e045b32739f5cad5c96900f6ed801844b3796
//META: purpose=A conformance produces a witness_table value whose IR type is witness_table_t(<interface>) with witness_table_entry rows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Witness-table types" table lists `witness_table_t`
// (parameterized by interface) as the type opcode for the value
// produced when a struct conforms to an interface. The conformance
// of `Square : IShape` emits
// `witness_table %... : witness_table_t(%IShape)(%Square)` with
// one `witness_table_entry` per requirement.

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
