//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey--builtinrequirementkey
//META: doc_section_digest=d80ae21dbc525a4a7b33400fe5616ab93a1f581227fecb5e8b1b0d8433863e1a
//META: purpose=A builtinRequirementKey is deduplicated by construction, so two distinct conformances to the same built-in interface reference the same key inst.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the hoistable-identity claim: the key's identity is its kind
// operand, so the same logical built-in requirement resolves to one inst.
// `GradA` and `GradB` each get their own witness table, and a table holds each
// requirement at most once. CHECK captures the numbered key of one
// `witness_table_entry` into a FileCheck variable and requires a later entry —
// necessarily in the other table — to name the identical inst.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float u;
uniform RWStructuredBuffer<float> buf;

struct GradA : IDifferentiable
{
    float a;
}

struct GradB : IDifferentiable
{
    float b;
}

[numthreads(1, 1, 1)]
void main()
{
    GradA x;
    x.a = u;
    GradB y;
    y.b = u;
    buf[0] = x.a + y.b;
}

// CHECK: witness_table_entry([[KEY:%[0-9]+]],
// CHECK: witness_table_entry([[KEY]],
