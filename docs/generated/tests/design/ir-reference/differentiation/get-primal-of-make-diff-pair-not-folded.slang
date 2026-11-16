//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#getdifferential--getprimal
//META: doc_section_digest=992c5be4c40a8e9d048876fb0fcf3e8d42db8a3550af8e540fa5fbb2049c019a
//META: purpose=The .p and .d getters applied to a DifferentialPair just built from MakeDiffPair lower to separate GetPrimal and GetDifferential projections of that pair; lowering does not fold them to the constructor operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit no-fold statement about the projection opcodes: even
// when a projection is applied to a pair the same block just constructed,
// `GetPrimal(MakeDiffPair(a, b))` is not reduced to `a`. The entry point builds
// the pair from two uniform globals and immediately reads both halves; the
// ordered CHECKs assert the dump still shows the construction followed by both
// projections applied to the named pair `%p`, which would be impossible if a
// peephole had folded them away at this stage.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    DifferentialPair<float> p = DifferentialPair<float>(a, d);
    buf[0] = p.p;
    buf[1] = p.d;
}

// CHECK: func %main
// CHECK: let %p{{.*}}= MakeDiffPair(%a, %d)
// CHECK: = GetPrimal(%p)
// CHECK: = GetDifferential(%p)
