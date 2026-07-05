//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
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
