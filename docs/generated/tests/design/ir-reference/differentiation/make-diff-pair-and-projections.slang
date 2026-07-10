//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#opcodes
//META: doc_section_digest=5251f06bb7f70c9893cfc402425163cd28989f883880155f2b7d9979f81db7f6
//META: purpose=A single entry point that constructs a DifferentialPair then reads both .p and .d exercises MakeDiffPair, GetPrimal, and GetDifferential in one IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Constructing `DifferentialPair<float>(a, d)` and projecting both
// `.p` and `.d` exercises the three pair opcodes in one body:
// `MakeDiffPair` (construction), `GetPrimal` (primal projection),
// and `GetDifferential` (differential projection).

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
    DifferentialPair<float> r = __fwd_diff(f)(DifferentialPair<float>(a, d));
    buf[0] = r.p;
    buf[1] = r.d;
}

// CHECK: func %main
// CHECK: MakeDiffPair(%a, %d)
// CHECK: GetPrimal(
// CHECK: GetDifferential(
