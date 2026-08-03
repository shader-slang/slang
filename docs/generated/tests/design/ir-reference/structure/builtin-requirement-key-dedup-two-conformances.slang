//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:18:00Z
//META: source_commit=ef4bd0b0d91a213b52e333ce39dfe8b73efca3d2
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey-builtinrequirementkey
//META: doc_section_digest=28df938d7b6c7f1da019467d16e8588fdfdf4490713d538aea7f26ee4dd9331f
//META: purpose=BuiltinRequirementKey is deduplicated by construction: two distinct conformances to the same built-in interface reference the same key inst in their witness_table_entry rows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies hoistable dedup of built-in requirement keys. `GradA` and
// `GradB` each conform to IDifferentiable, so each gets its own
// witness_table — but the same logical built-in requirement must resolve
// to ONE key inst. Built-in keys carry no name hint, so they print as
// numbered values; the CHECKs capture the numbered key of one
// witness_table_entry and require a second entry (necessarily in a
// different table, since a table holds each requirement once) to
// reference the identical inst. If dedup broke, the second table's
// entries would name a fresh key and the second match would fail.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float u;
uniform RWStructuredBuffer<float> buf;

struct GradA : IDifferentiable
{
    float a;
};

struct GradB : IDifferentiable
{
    float b;
};

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
