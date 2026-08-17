//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=fdd06f0a3a11b54325a8474139094f83e3f6174898d99c25f5f4cdcfc864d79a
//META: purpose=Boundary: a function returning a user struct names that struct as the Func result-type operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a function returning a user struct names that struct as the Func
// result-type operand. When the function returns a user struct, the
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
