//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:20+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=a5f429e50a404aa0a4ddb2be899ebf5ee6430f639bfac92c88add9f4cdae7ee3
//META: purpose=no_diff applied to a call expression attaches TreatCallAsDifferentiableDecoration to the call and emits no detachDerivative.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the call flavour of `no_diff` on an expression, which the
// detachDerivative section singles out as the exception to the load-wrapping
// rule. `no_diff(g(x))` wraps a call, so the lowering visitor decorates the
// call itself instead of wrapping its result. CHECK requires the decoration to
// sit directly above the `call %g` inside `func %f`, and the CHECK-NOT after
// the call forbids a `detachDerivative` anywhere in the function body before
// the module-scope derivative request that follows it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

float g(float x) { return x * 3.0; }

[Differentiable]
float f(float x)
{
    return x * no_diff(g(x));
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, d)).d;
}

// CHECK: func %f{{.*}}: Func(Float, Float)
// CHECK-NOT: detachDerivative
// CHECK: [treatCallAsDifferentiableDecoration]
// CHECK-NEXT: let {{%[A-Za-z0-9_]+}} : Float = call %g(
// CHECK-NOT: detachDerivative
// CHECK: = ForwardDifferentiate(%f)
