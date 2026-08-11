//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=9c6b7a39271eb0f667fd936efc160c0082114c1a92422a72493cec6e8b0fc6cb
//META: purpose=The free function diffPair(primal, diff) is a second user spelling of MakeDiffPair and yields the same opcode as the DifferentialPair<T> constructor.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the claim that the construction opcode is reachable from ordinary
// user code through a second spelling — the core-module free function
// `diffPair(primal, diff)`. The pair is bound to a named local so the dump line
// carries the `nameHint("p")`; CHECK asserts that this named binding is a
// `MakeDiffPair(%a, %d)` value with the same `DiffPair(Float, ...)` result type
// the constructor spelling produces, which is what makes the two spellings
// interchangeable at IR level.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x + x * x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> p = diffPair(a, d);
    buf[0] = __fwd_diff(f)(p).d;
}

// CHECK: func %main
// CHECK: let %p{{.*}}DiffPair(Float, {{%[A-Za-z0-9_]+}}){{.*}}= MakeDiffPair(%a, %d)
