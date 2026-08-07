//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#differential-pair-projection
//META: doc_section_digest=5f50033280aec505eacac53ad283962baab159487ef0677b633a1b40a4acdceb
//META: purpose=Both the .p and the .v getter of DifferentialPair<T> lower to GetPrimal on the pair, producing the primal element type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `DifferentialPair<T>` exposes two source spellings of the same
// primal projection and that both map onto the single `GetPrimal` opcode. The
// entry point reads `.p` from one returned pair and `.v` from a second one and
// sinks both into the output buffer so DCE keeps them; a repeat-count assertion
// then requires two `Float = GetPrimal(...)` lines after `func %main` — one per
// getter — which only holds if `.v` is an alias of `.p` rather than a distinct
// opcode.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float f(float x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> r = __fwd_diff(f)(DifferentialPair<float>(a, d));
    DifferentialPair<float> s = __fwd_diff(f)(DifferentialPair<float>(d, a));
    buf[0] = r.p;
    buf[1] = s.v;
}

// CHECK: func %main
// CHECK-COUNT-2: Float{{.*}}= GetPrimal(
