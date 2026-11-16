//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#builtinrequirementkey
//META: doc_section_digest=2ceef85ef0f7e3e2032b23321c6a33649940568aac23a65e0680b6a547265147
//META: purpose=Every builtinRequirementKey is emitted with a BuiltinRequirementDecoration carrying the same requirement-kind operand, so a lookup can find a witness-table entry by role instead of by position.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the key/decoration pairing that makes role-based requirement lookup
// possible. Using a differentiable function pulls the recognized
// `IDifferentiable` requirements into the module, and each key appears in the
// dump directly under its decoration (the dump currently prints the decoration
// line twice for each key, so the pattern does not pin the line count). CHECK
// captures the kind operand from a decoration line, CHECK-NOT forbids any
// intervening `let`, and the next CHECK requires the key inst built from that
// same kind, which is the invariant a by-role lookup depends on; a decoration
// carrying a different kind than its key would break it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: {{^}}[BuiltinRequirementDecoration([[KIND:[0-9]+]] : Int)]
// CHECK-NOT: let
// CHECK: let {{%[A-Za-z0-9_]+}} : _ = builtinRequirementKey([[KIND]] : Int)
