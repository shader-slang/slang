//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=f9ced424f75b0995671347e9ab72c4c596438a42cdf6bd9ecf359a2b13da7f4e
//META: purpose=An integer division expression lowers to the `div` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `div`/`Div` value-instruction row. Two non-constant
// `uniform` operands are divided so the IR dump shows `div(%a, %b)`
// instead of a folded constant.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a / b;
}

// IR-LABEL: func %main
// IR: div(%{{.*}}, %{{.*}})
