//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#detachderivative
//META: doc_section_digest=01132bdcdbc1c50944722b06abe5d963b03cd1175d44d81a16fc40d947d9c916
//META: purpose=no_diff on a parameter declaration is a modifier rather than an expression and produces no detachDerivative inst at all.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary between the two `no_diff` spellings. The sibling test
// `no-diff-expression-detach.slang` shows `no_diff(expr)` producing a
// `detachDerivative`; here the same keyword sits on a parameter declaration
// instead, and the claim is that this form produces none. CHECK binds the
// module-scope `no_diff` value, requires `k`'s parameter type to be
// `Attributed(Int, <that value>)` — which is the modifier surviving as part of
// the type rather than as an inst — and brackets the LOWER-TO-IR section with
// `CHECK-NOT: detachDerivative`. The entry point differentiates `scaleBy`, so
// the autodiff machinery really does run over this parameter; the absence is
// therefore about the spelling, not about the function being ignored.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform int n;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float scaleBy(float x, no_diff int k)
{
    return x * float(k);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(scaleBy)(DifferentialPair<float>(a, d), n).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK-NOT: detachDerivative
// CHECK: let [[ND:%[A-Za-z0-9_]+]] : Void = no_diff
// CHECK-NOT: detachDerivative
// CHECK: func %scaleBy : Func(Float, Float, Attributed(Int, [[ND]]))
// CHECK: param %k : Attributed(Int, [[ND]])
// CHECK-NOT: detachDerivative
// CHECK: {{^}}###{{$}}
