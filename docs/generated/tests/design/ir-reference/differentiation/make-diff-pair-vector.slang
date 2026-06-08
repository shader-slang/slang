//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=1ed9b2d6eca0e76cbc4a138fe92fc22ce6b01a2c4f3f5f4084d95d7228a1b421
//META: purpose=MakeDiffPair works on vector primal/differential types; result type is DiffPair(Vec(...), witness).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `MakeDiffPair` is element-type-polymorphic. With a `float3` primal,
// the result type at LOWER-TO-IR is `DiffPair(Vec(Float, 3), %w)`
// (the vector type is spelled `Vec(elementT, N)` in the dump).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float3 a;
uniform float3 d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float3 f(float3 x)
{
    return x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float3>(a, d)).d.x;
}

// CHECK: DiffPair(Vec(Float, 3 : Int),{{.*}}){{.*}}= MakeDiffPair(%a, %d)
