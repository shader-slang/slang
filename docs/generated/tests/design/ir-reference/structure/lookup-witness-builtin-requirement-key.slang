//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:18:00Z
//META: source_commit=ef4bd0b0d91a213b52e333ce39dfe8b73efca3d2
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey-builtinrequirementkey
//META: doc_section_digest=28df938d7b6c7f1da019467d16e8588fdfdf4490713d538aea7f26ee4dd9331f
//META: purpose=Either requirement-key kind can appear as the lookupKey of lookupWitness: dispatching a built-in requirement (IDifferentiable's dzero) through a constrained type parameter looks the key up with a builtinRequirementKey.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a builtinRequirementKey flows through witness dispatch.
// In `zeroOf<T : IDifferentiable>`, both the `T.Differential` result type
// and the `T.dzero()` call must be looked up in T's witness table, and the
// only requirement keys IDifferentiable offers are built-in ones. The
// CHECK-DAGs assert the dump holds a `builtinRequirementKey(` definition
// and a `lookupWitness` whose key operand is an unnamed numbered inst
// (a StructKey key would print with its mangled x5F name) — together
// showing the built-in key kind occupying the lookupKey slot.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

T.Differential zeroOf<T : IDifferentiable>()
{
    return T.dzero();
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = zeroOf<float>();
}

// CHECK-DAG: = builtinRequirementKey(
// CHECK-DAG: lookupWitness(%{{[0-9]+}}, %{{[0-9]+}})
