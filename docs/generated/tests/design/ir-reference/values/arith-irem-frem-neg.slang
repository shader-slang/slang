//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Integer `%` lowers to `irem`, floating-point `%` lowers to `frem`, and prefix `-` lowers to `neg`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `%` picks its opcode from the *element* type of its
// operands and that unary `-` has its own opcode. Integer `%` must
// become `irem` (remainder, not modulus), floating-point `%` must
// become `frem`, and prefix `-` must become `neg`; CHECK asserts all
// three on `uniform` operands so none of them folds away.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a % b;
    fbuf[0] = fa % fb;
    buf[1] = -a;
}

// IR-LABEL: func %main
// IR: irem(%a, %b)
// IR: frem(%fa, %fb)
// IR: neg(%a)
