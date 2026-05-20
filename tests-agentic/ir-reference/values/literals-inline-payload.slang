//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
//META: purpose=Constant-family opcodes (IntLit / FloatLit / BoolLit / VoidLit) carry their value inline on the IRInst and appear as `<value> : <Type>` operands rather than as separately-named lines.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A handful of literal operands in arithmetic / store / return
// positions surface the inline-payload encoding the doc describes:
// integer literals as `<N> : Int`, float as `<F> : Float`, bool as
// `true` / `false`, and the unique `void_constant` value as the
// return operand of a `void` function.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float fa;
uniform int x;
uniform int y;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 42;
    fbuf[0] = fa + 1.5;
    buf[1] = select(true, x, y);
    buf[2] = select(false, x, y);
}

// IR-LABEL: func %main
// IR: add(%a, 42 : Int)
// IR: add(%fa, 1.5 : Float)
// IR: select(true, %x, %y)
// IR: select(false, %x, %y)
// IR: return_val(void_constant)
