//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:18:00Z
//META: source_commit=ef4bd0b0d91a213b52e333ce39dfe8b73efca3d2
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey-builtinrequirementkey
//META: doc_section_digest=28df938d7b6c7f1da019467d16e8588fdfdf4490713d538aea7f26ee4dd9331f
//META: purpose=Conforming a user struct to the built-in interface IDifferentiable produces builtinRequirementKey insts as the requirement keys, alongside the conformance's witness_table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a recognized built-in requirement is keyed by the
// `builtinRequirementKey` opcode rather than an ordinary named `key`.
// `struct Grad : IDifferentiable` forces the user module to materialize
// requirement keys for IDifferentiable's members (Differential / dzero /
// dadd, which the doc names as built-in); the CHECK-DAGs assert the dump
// contains a `builtinRequirementKey(` definition and the witness_table
// for the `%Grad` conformance those keys belong to.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float u;
uniform RWStructuredBuffer<float> buf;

struct Grad : IDifferentiable
{
    float v;
};

[numthreads(1, 1, 1)]
void main()
{
    Grad g;
    g.v = u;
    buf[0] = g.v;
}

// CHECK-DAG: = builtinRequirementKey(
// CHECK-DAG: witness_table %{{[0-9]+}}{{.*}}(%Grad)
