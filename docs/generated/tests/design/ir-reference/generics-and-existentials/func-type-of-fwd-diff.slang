//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization
//META: doc_section_digest=281f505f04ffec7b524f03c8f78892260d45c554e3e06b9a86d32cfe7c6ec9c0
//META: purpose=A func-dependent derivative type lowers to FuncTypeOf(callable) — a Type-valued projection that names a callable without embedding it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the compile-time projection a func-dependent type uses. Calling
// `fwd_diff` on a `[Differentiable]` function makes the derivative's type
// depend on the original callable, and lowering names that callable through a
// projection instead of embedding it. The CHECK asserts the projection's result
// is a `Type`.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float inVal;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float square(float x) { return x * x; }

[numthreads(1, 1, 1)]
void main()
{
    var dp = fwd_diff(square)(DifferentialPair<float>(inVal, 1.0));
    buf[0] = dp.d;
}

// CHECK: let  %{{[0-9]+}} : Type = FuncTypeOf(%{{[A-Za-z0-9_]+}})
