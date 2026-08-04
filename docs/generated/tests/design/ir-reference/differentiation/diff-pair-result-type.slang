//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:32:24+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#makediffpair
//META: doc_section_digest=1ed9b2d6eca0e76cbc4a138fe92fc22ce6b01a2c4f3f5f4084d95d7228a1b421
//META: purpose=MakeDiffPair produces a value of type DiffPair(T, witness); the witness operand is the IDifferentiable witness for T.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The result type of `MakeDiffPair(%primal, %diff)` is spelled
// `DiffPair(T, %witness)` in the IR-dump form: the first operand is
// the primal element type and the second operand is the
// `IDifferentiable` witness for that type. This is the cross-link
// between the construction opcode and the differential-pair type
// family.

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

// CHECK: DiffPair(Float, {{%[A-Za-z0-9_]+}}){{.*}}= MakeDiffPair(
