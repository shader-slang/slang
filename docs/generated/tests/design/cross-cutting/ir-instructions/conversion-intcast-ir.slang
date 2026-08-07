//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=180e0aa25246196c89b6dc01c680b751b33a813ef1597e4ac18dd9fef40c0943
//META: purpose=An explicit integer-width cast lowers to the `intCast` conversion opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's conversion-opcode row (`bitCast`, `intCast`,
// `floatCast`). A constructor-style `int64_t(a)` on a non-constant
// `int` produces the `intCast` opcode in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int64_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int64_t(a);
}

// IR-LABEL: func %main
// IR: intCast(%{{.*}})
