//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:12:37+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#built-in-requirement-keys
//META: doc_section_digest=f9fe48ae137d96b72713ab03d2eba0f40566c196098995d57df4313cf4bee2c7
//META: purpose=Lowering assigns hoistable BuiltinRequirementKey insts as the requirement keys of the built-in IDifferentiable-family interfaces, visible in the LOWER-TO-IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the requirement keys of the built-in IDifferentiable
// family are `BuiltinRequirementKey` insts already at LOWER-TO-IR
// (AST origin "(lowering)", not an autodiff-pass product). A
// `[ForwardDifferentiable]` function pulls the IDifferentiable
// machinery into the module; CHECK matches the key inst's definition
// spelling `= builtinRequirementKey(` (the opcode's dump name), which
// only a lowering-assigned built-in key produces (an ordinary
// requirement key dumps as `key`).
// The bare opcode substring cannot false-positive elsewhere in the
// large autodiff preamble.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float g(float x)
{
    return 3.0 * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(g)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: = builtinRequirementKey(
