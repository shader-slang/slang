//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#builtinrequirementkey
//META: doc_section_digest=82acd463d8cb41f494c18915dac62b4eeb436a684f415f6a51bfca02f2381039
//META: purpose=The requirement role is promoted from DifferentialType to DifferentialWitness when the requirement being keyed is the associated conformance rather than the associated type, so the two get distinct keys.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the promotion the section describes, which is what stops the
// associated type `Differential` and the conformance `Differential :
// IDifferentiable` from colliding on one key. A user struct conforming to
// `IDifferentiable` produces a witness table containing both requirements at
// once, so a single dump shows the pair. CHECK captures the two keys by the
// kind operand they were constructed from — 1 for `DifferentialType` and 27
// for `DifferentialWitness` in `BuiltinRequirementKind` order — and then
// requires the `MyVec` witness table to store the type under the first key and
// the witness table itself under the second. Binding the table to a variable
// and requiring the conformance entry's value to be that same variable is the
// point: `MyVec` is its own `Differential`, so if the two roles shared a key
// the entries would be indistinguishable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

struct MyVec : IDifferentiable
{
    float x;
    float y;
}

[Differentiable]
float sum(MyVec v)
{
    return v.x + v.y;
}

[numthreads(1, 1, 1)]
void main()
{
    MyVec p = { a, a };
    MyVec dp = { d, d };
    buf[0] = __fwd_diff(sum)(DifferentialPair<MyVec>(p, dp)).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: let [[DIFFTYPEKEY:%[A-Za-z0-9_]+]] : _ = builtinRequirementKey(1 : Int)
// CHECK: let [[DIFFWITKEY:%[A-Za-z0-9_]+]] : _ = builtinRequirementKey(27 : Int)
// CHECK: witness_table [[WT:%[A-Za-z0-9_]+]] : witness_table_t(%IDifferentiable)(%MyVec)
// CHECK: witness_table_entry([[DIFFTYPEKEY]],%MyVec)
// CHECK: witness_table_entry([[DIFFWITKEY]],[[WT]])
