//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#getdifferential--getprimal
//META: doc_section_digest=e0caf7b12e1402f83e7150059675cfa7cf9b9e8e42a3d9fade7e3c1e4e5f18f7
//META: purpose=No pass folds GetPrimal or GetDifferential applied directly to a MakeDiffPair back to the constructor operand before the pair-lowering rewrite.
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

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

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
