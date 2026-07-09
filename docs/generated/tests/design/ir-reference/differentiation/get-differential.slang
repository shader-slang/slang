//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-projection
//META: doc_section_digest=c901479d85106c278176dc14d6b22407f46e5dc12a769c4118d6b8671da0d2f1
//META: purpose=Reading .d off a returned DifferentialPair lowers to GetDifferential(%pair).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `.d` projection on a `DifferentialPair<float>` value lowers to a
// `GetDifferential(%pair)` IR value whose result type is the
// differential type (Float for float).

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
