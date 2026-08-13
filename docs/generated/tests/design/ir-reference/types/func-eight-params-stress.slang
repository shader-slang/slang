//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=fdd06f0a3a11b54325a8474139094f83e3f6174898d99c25f5f4cdcfc864d79a
//META: purpose=Stress: eight-parameter function preserves all parameter types as positional operands of Func.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// eight-parameter function preserves all parameter types as positional
// operands of Func. A function with eight `int` parameters surfaces as
// `Func(Int, Int, Int, Int, Int, Int, Int, Int, Int)` — one result
// plus eight params.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int many(int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7)
{
    return p0 + p1 + p2 + p3 + p4 + p5 + p6 + p7;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = many(a, a, a, a, a, a, a, a);
}

// IR-LABEL: func %many
// IR-SAME: Func(Int, Int, Int, Int, Int, Int, Int, Int, Int)
