//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=Boundary: prefix `-` on a `uniform int` (whose value spans the full signed-32 range, including the documented MIN where unary negation overflows) lowers to `neg(%a)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The neg opcode is exercised in the smoke test through compound
// expressions. This boundary probe isolates `-a` on a uniform —
// the `a` operand's value is unknown to the compiler, so the
// `neg(MIN)` overflow edge is preserved as a single `neg`
// instruction. Lowering does not pre-bake the overflow path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = -a;
}

// IR-LABEL: func %main
// IR: neg(%a)
