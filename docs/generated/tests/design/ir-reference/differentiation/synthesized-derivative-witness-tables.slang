//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#synthesized-derivative-witnesses
//META: doc_section_digest=4ac46248bc67dd90a58b5dbd80b92929f4d02dd326a0ef7823411d0e2a772c21
//META: purpose=The derivative of a differentiable function is itself given IForwardDifferentiable and IBackwardDifferentiable witness tables through SynthesizedForwardDerivativeWitnessTable and SynthesizedBackwardDerivativeWitnessTable.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two witness-synthesis rows and the reason they exist: a
// higher-order derivative needs the first derivative to itself be
// differentiable, so lowering emits a witness-table request for each direction
// over the derivative value rather than over the original function. CHECK binds
// the forward-derivative value produced from `%f` and then requires both
// witness-table requests to take that bound value as their operand, which is
// what distinguishes "witness for the derivative" from "witness for f".

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, d)).d;
}

// CHECK: func %f{{.*}}: Func(Float, Float)
// CHECK: let [[FWD:%[A-Za-z0-9_]+]] :{{.*}}= ForwardDifferentiate(%f)
// CHECK: = SynthesizedForwardDerivativeWitnessTable([[FWD]])
// CHECK: = SynthesizedBackwardDerivativeWitnessTable([[FWD]])
