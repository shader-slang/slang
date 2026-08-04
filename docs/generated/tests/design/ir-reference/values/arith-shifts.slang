//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=The `<<` operator lowers to `shl` and `>>` lowers to `shr`; the doc notes the arithmetic-vs-logical choice is keyed by operand signedness.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two shift operators, on signed and unsigned integers, both
// surface the `shl` / `shr` opcodes the doc lists in the Arithmetic
// and bitwise table.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform uint ua;
uniform int amt;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<uint> ubuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a << amt;
    buf[1] = a >> amt;
    ubuf[0] = ua << amt;
    ubuf[1] = ua >> amt;
}

// IR-LABEL: func %main
// IR: shl(%a, %amt)
// IR: shr(%a, %amt)
// IR: shl(%ua, %amt)
// IR: shr(%ua, %amt)
