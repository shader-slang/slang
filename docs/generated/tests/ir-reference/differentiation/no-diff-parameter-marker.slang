//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#opcodes
//META: doc_section_digest=aacf05fed4ed4ce34369f80555096390dd0ba483e842c1405e66f4d2a320bfb7
//META: purpose=A no_diff parameter surfaces as Attributed(T, %no_diff) on the function entry block at LOWER-TO-IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `no_diff T x` parameter on a `[ForwardDifferentiable]` function
// surfaces at LOWER-TO-IR as `param %x : Attributed(T, %no_diff)` on
// the function's entry block. The `%no_diff` referenced in the
// wrapper is a module-scope `let %k : Void = no_diff` value, which
// also surfaces as a top-level declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float c;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(no_diff float k, float x)
{
    return k * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(c, DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: : Void	= no_diff
// CHECK: param %k	: Attributed(Float,
