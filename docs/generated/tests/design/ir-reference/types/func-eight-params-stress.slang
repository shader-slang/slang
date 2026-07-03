//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=021d788619e4517ac1c3910bae98363f85bfa77ed12c6b9a50b7bd8e1f64f627
//META: purpose=Stress: eight-parameter function preserves all parameter types as positional operands of Func.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Func` note says the remaining operands after the
// result type are parameter types "in declaration order". A
// function with eight `int` parameters surfaces as
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
