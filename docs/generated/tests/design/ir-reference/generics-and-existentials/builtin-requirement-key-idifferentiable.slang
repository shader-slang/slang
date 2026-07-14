//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:08:34Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#builtinrequirementkey-builtinrequirementkey
//META: doc_section_digest=d1fa25ce7ebb01fb9d1853fcc53938fdbbb6f34baa348e767217ad026e5899b0
//META: purpose=A struct conforming to IDifferentiable produces witness-table rows keyed by builtinRequirementKey insts carrying an integer BuiltinRequirementKind operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that requirements the front end recognizes as built-in
// interface members (the IDifferentiable roles) are keyed by
// builtinRequirementKey rather than by a named StructKey. A struct
// declared as conforming to IDifferentiable is routed through a
// constrained generic so its witness table stays live; the CHECKs pin
// the table for the user struct plus at least one builtinRequirementKey
// whose sole operand is an integer literal (the BuiltinRequirementKind).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct DiffData : IDifferentiable { float v; };

uniform float inVal;
uniform RWStructuredBuffer<float> buf;

float useDiff<T : IDifferentiable>(T t, float x) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    DiffData d;
    d.v = inVal;
    buf[0] = useDiff<DiffData>(d, inVal);
}

// CHECK-DAG: witness_table %{{[0-9]+}} : witness_table_t(%{{[A-Za-z0-9_]+}})(%DiffData)
// CHECK-DAG: builtinRequirementKey({{[0-9]+}} : Int)
