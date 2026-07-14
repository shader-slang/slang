//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=4ef79db8cd0b400370a0c8b9af0f015486f7700806d087a1543bda7c79ef3d9a
//META: purpose=detach(x) inside a differentiable function lowers to detachDerivative(%x).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `detach(x)` inside a `[ForwardDifferentiable]` body lowers to a
// `detachDerivative(%x)` IR value whose result type is the operand
// type and whose semantics return the operand unchanged but block
// derivative propagation.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    // detach(x) blocks derivative propagation through the first factor;
    // the derivative of f comes only through the second `x`.
    return detach(x) * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: func %f
// CHECK: Float{{.*}}= detachDerivative(%x)
