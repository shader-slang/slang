//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey--builtinrequirementkey
//META: doc_section_digest=d4cd5719f0455c4590eab034c86ccb9dfcac298707fabbcaa0455bcade824d2d
//META: purpose=Conforming a user struct to the built-in interface IDifferentiable produces builtinRequirementKey insts as the requirement keys, alongside the conformance's witness_table.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a recognized built-in requirement is keyed by the
// `builtinRequirementKey` opcode rather than by an ordinary named `key`.
// `struct Grad : IDifferentiable` forces the user module to materialize
// requirement keys for IDifferentiable's built-in members; the CHECK-DAGs
// require the dump to hold a `builtinRequirementKey(` definition and the
// `%Grad` conformance table those keys belong to.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float u;
uniform RWStructuredBuffer<float> buf;

struct Grad : IDifferentiable
{
    float v;
}

[numthreads(1, 1, 1)]
void main()
{
    Grad g;
    g.v = u;
    buf[0] = g.v;
}

// CHECK-DAG: = builtinRequirementKey(
// CHECK-DAG: witness_table %{{[0-9]+}}{{.*}}(%Grad)
