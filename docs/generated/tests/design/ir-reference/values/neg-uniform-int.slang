//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
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
