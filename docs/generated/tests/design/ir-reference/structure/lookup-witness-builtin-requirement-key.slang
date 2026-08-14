//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#builtinrequirementkey--builtinrequirementkey
//META: doc_section_digest=d4cd5719f0455c4590eab034c86ccb9dfcac298707fabbcaa0455bcade824d2d
//META: purpose=Either requirement-key kind can occupy the lookupKey slot of witness dispatch, so a built-in requirement is looked up with a builtinRequirementKey.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies why the `lookupKey` operand is typed as a plain inst rather than a
// struct key. In `zeroOf<T : IDifferentiable>` both the `T.Differential` result
// type and the `T.dzero()` call must be resolved through T's witness table, and
// IDifferentiable offers only built-in requirements. The CHECK-DAGs require a
// `builtinRequirementKey(` definition plus a `lookupWitness` whose key operand
// is an unnamed numbered inst — a struct key would print with its mangled x5F
// name instead.

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
