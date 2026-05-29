//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#legacy-bridge
//META: doc_section_digest=595a918da93f7045a60ed099e5b1505eabfb7a1ad81646df40e38b2957839d55
//META: purpose=The unprefixed bwd_diff(f) form also lowers to LegacyBackwardDifferentiate at LOWER-TO-IR, same as __bwd_diff.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `__bwd_diff(...)` as the AST origin of the
// reverse-mode differentiation operator. At LOWER-TO-IR, both
// `__bwd_diff(f)` and the unprefixed alias `bwd_diff(f)` lower to
// `LegacyBackwardDifferentiate(...)` (the modern opcode is
// synthesized only later by the unzip pass).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[BackwardDifferentiable]
float f(float x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    var dpx = DifferentialPair<float>(a, 0.0);
    bwd_diff(f)(dpx, 1.0);
    buf[0] = dpx.d;
}

// CHECK: LegacyBackwardDifferentiate(
