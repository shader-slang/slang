//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:12:37+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#built-in-requirement-keys
//META: doc_section_digest=f9fe48ae137d96b72713ab03d2eba0f40566c196098995d57df4313cf4bee2c7
//META: purpose=Each built-in requirement key is tagged with a BuiltinRequirementDecoration recording the BuiltinRequirementKind role it represents.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lowering tags the built-in requirement keys with the
// BuiltinRequirement decoration (the role marker the autodiff lookup
// helper scans, instead of relying on entry position). The dump prints
// decorations bracketed above the decorated inst, so an ordered CHECK
// pins a `[BuiltinRequirement...]` decoration line followed by the
// `= builtinRequirementKey(` definition it decorates (the opcode's
// dump name); the bracket prefix keeps the first pattern from
// matching the definition line itself.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float h(float x)
{
    return x + x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(h)(DifferentialPair<float>(a, 1.0)).d;
}

// The decoration line precedes the key definition it marks, so plain
// ordered CHECKs are the property under test here (not CHECK-DAG).
// CHECK: [BuiltinRequirement
// CHECK: = builtinRequirementKey(
