//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forwarddifferentiate
//META: doc_section_digest=c21b0ae05c57902d66f656e2476fa3282420702de64bd927d7d6f0471493563c
//META: purpose=__fwd_diff(f) lowers to a single-operand ForwardDifferentiate whose operand is the base function and whose result type is the JVP function type over differential pairs.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__fwd_diff` is the one differentiation operator reachable
// directly from a user expression, and that the request it produces names the
// base function. The dump shows the request as a module-scope value; CHECK
// binds that value to a FileCheck variable on the line whose producer is
// `ForwardDifferentiate(%f)` and whose type is `Func(DiffPair(...),
// DiffPair(...))` — the JVP signature over differential pairs — then requires
// the call inside `func %main` to target exactly that value, which is what
// makes the request the callee rather than `%f` itself.

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
// CHECK: let [[FWD:%[A-Za-z0-9_]+]] : Func(DiffPair(Float, {{%[A-Za-z0-9_]+}}), DiffPair(Float, {{%[A-Za-z0-9_]+}})) = ForwardDifferentiate(%f)
// CHECK: func %main
// CHECK: call [[FWD]](
