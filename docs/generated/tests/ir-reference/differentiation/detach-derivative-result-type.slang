//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=4ef79db8cd0b400370a0c8b9af0f015486f7700806d087a1543bda7c79ef3d9a
//META: purpose=detachDerivative returns its operand unchanged with the same scalar result type at LOWER-TO-IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states `detachDerivative(value)` returns the operand
// unchanged but blocks derivative propagation. The result type
// equals the operand type — for a `float` operand the IR-dump
// result line is `let %N : Float = detachDerivative(%x)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float blockDeriv(float x)
{
    return detach(x);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(blockDeriv)(DifferentialPair<float>(a, 1.0)).d;
}

// CHECK: func %blockDeriv
// CHECK: Float	= detachDerivative(%x)
