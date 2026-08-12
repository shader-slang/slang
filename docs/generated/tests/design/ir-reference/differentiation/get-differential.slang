//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-projection
//META: doc_section_digest=5f50033280aec505eacac53ad283962baab159487ef0677b633a1b40a4acdceb
//META: purpose=The .d getter of DifferentialPair<T> lowers to GetDifferential on the pair, producing the differential element type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the differential half of the projection table: reading `.d` off a
// `DifferentialPair<float>` returned by a forward-differentiated call emits
// `GetDifferential`, whose result type is the differential type of `float` —
// `Float` again, since the scalar is its own differential. CHECK is anchored
// after `func %main` so the many `GetDifferential` uses in the core-module
// autodiff preamble cannot satisfy the pattern.

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

// CHECK: func %main
// CHECK: Float{{.*}}= GetDifferential(
