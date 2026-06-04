//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=021d788619e4517ac1c3910bae98363f85bfa77ed12c6b9a50b7bd8e1f64f627
//META: purpose=Boundary: a function returning a user struct names that struct as the Func result-type operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Func` note says the result type is the leading
// operand. When the function returns a user struct, the
// `Func(%Pair, Int)` form names the struct's IR identity in the
// result-type slot — distinct from the basic-scalar smoke test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int a; int b; }

uniform int x;
uniform RWStructuredBuffer<int> buf;

Pair makePair(int v) { Pair p; p.a = v; p.b = v + 1; return p; }

[numthreads(1, 1, 1)]
void main()
{
    Pair p = makePair(x);
    buf[0] = p.a + p.b;
}

// IR-LABEL: func %makePair
// IR-SAME: Func(%Pair, Int)
