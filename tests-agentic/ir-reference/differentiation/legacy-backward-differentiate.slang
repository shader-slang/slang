//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/ir-reference/differentiation.md#legacy-bridge
//META: doc_section_digest=595a918da93f7045a60ed099e5b1505eabfb7a1ad81646df40e38b2957839d55
//META: purpose=__bwd_diff(f) on a [BackwardDifferentiable] function lowers at LOWER-TO-IR to LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t), not to BackwardDifferentiate.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__bwd_diff(f)` on a `[BackwardDifferentiable]` function lowers to a
// `LegacyBackwardDifferentiate` IR value whose three operands are the
// bwd-callable function, the remat function, and the intermediate-
// context type. The doc lists `BackwardDifferentiate` as the AST
// origin of `__bwd_diff`, but the actual LOWER-TO-IR opcode is the
// legacy bridge form; the unzip pass later converts it to the
// modern triple. Recorded as a doc gap.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[BackwardDifferentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    var dpx = DifferentialPair<float>(a, 0.0);
    __bwd_diff(f)(dpx, 1.0);
    buf[0] = dpx.d;
}

// CHECK: LegacyBackwardDifferentiate(
