//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=1ed9b2d6eca0e76cbc4a138fe92fc22ce6b01a2c4f3f5f4084d95d7228a1b421
//META: purpose=DifferentialPair<T>(primal, diff) constructor at a __fwd_diff call site lowers to MakeDiffPair(%primal, %diff).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The user-written `DifferentialPair<float>(a, b)` constructor at a
// __fwd_diff(...) call site lowers to a `MakeDiffPair(%a, %b)` IR value
// whose result type is `DiffPair(Float, %witness)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(f)(DifferentialPair<float>(a, d)).d;
}

// CHECK: func %main
// CHECK: DiffPair(Float, {{%[A-Za-z0-9_]+}}){{.*}}= MakeDiffPair(%a, %d)
