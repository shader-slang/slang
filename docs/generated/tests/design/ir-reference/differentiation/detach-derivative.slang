//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=6a606134e98de20cf82c1e5baedde0a38a9f404df32219b204c2dad2b81869b7
//META: purpose=The core-module detach<T>(T x) intrinsic lowers to a detachDerivative inst whose single operand is the value being detached and whose result type is that value's type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the first of the two documented source spellings of the opcode: the
// `detach<T>` intrinsic. Inside the differentiable function the parameter `%x`
// is fed to `detach` and the result is multiplied back with `%x`, so the dump
// keeps both the detached and the undetached use of the same value. CHECK
// requires the detach line to name `%x` directly as its operand and to carry
// the operand's own type `Float`, which is the "returns its operand unchanged"
// half of the claim at IR level.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float f(float x)
{
    return x * detach(x);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, d)).d;
}

// CHECK: func %f{{.*}}: Func(Float, Float)
// CHECK: let [[DET:%[A-Za-z0-9_]+]] : Float = detachDerivative(%x)
// CHECK: = mul(%x, [[DET]])
