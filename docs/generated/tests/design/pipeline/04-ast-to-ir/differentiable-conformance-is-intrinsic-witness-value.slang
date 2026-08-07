//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#differentiability-arrives-as-a-conformance
//META: doc_section_digest=34ead772bac23de346f65addfa530c4a7fd4069f6482eaad6257c9095f426f4c
//META: purpose=A `SynthesizedModifier`-tagged differentiability conformance lowers to one intrinsic inst typed `witness_table_t(...)` carrying the conformance's operands, instead of a real `IRWitnessTable` with requirement entries.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the synthesized-conformance shortcut. The higher-order conformance
// of the derivative function `sq.fwd_diff` to `IForwardDifferentiable` is
// synthesized, so lowering emits a value whose type is a witness-table type but
// whose definition is a single operand-carrying inst — back-end passes rebuild
// a table from the operands when one is needed. The CHECK requires exactly that
// shape: a `let` binding typed `witness_table_t(...)` whose right-hand side is
// an intrinsic applied to the derivative function, not a `witness_table` block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float scale(float x)
{
    return x * 3.0f;
}

[numthreads(1, 1, 1)]
void main()
{
    var dp = DifferentialPair<float>(a, 1.0);
    buf[0] = fwd_diff(scale)(dp).d;
}

// CHECK: [nameHint("scale.fwd_diff")]
// CHECK-NEXT: let [[FWD:%[A-Za-z0-9_]+]] : Func({{.*}}) = ForwardDifferentiate(%scale)
// CHECK: let {{.*}} : witness_table_t({{.*}}) = SynthesizedForwardDerivativeWitnessTable([[FWD]])
