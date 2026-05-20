//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#witness-table-types
//META: doc_section_digest=06b14fdf48b80a42b7d80961315e045b32739f5cad5c96900f6ed801844b3796
//META: purpose=Stress: two distinct conformances to the same interface produce two distinct witness_table values parameterised by the same interface.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `witness_table_t(baseType: IRType)` is parameter-
// ised by interface. Two struct conformances against the same
// interface produce two distinct `witness_table` values whose
// types both share `witness_table_t(%IShape)` but differ in the
// concrete-type parenthesised application.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { float area(); }
struct Circle : IShape { float r; float area() { return 3.14 * r * r; } }
struct Square : IShape { float s; float area() { return s * s; } }

uniform float r;
uniform float s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    Circle c = { r };
    Square sq = { s };
    buf[0] = c.area() + sq.area();
}

// IR: witness_table %{{.*}}: witness_table_t(%IShape)(%Circle)
// IR: witness_table %{{.*}}: witness_table_t(%IShape)(%Square)
