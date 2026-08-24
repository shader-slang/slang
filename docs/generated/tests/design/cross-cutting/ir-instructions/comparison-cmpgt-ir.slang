//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=31b4dd520f90edc4b02037b40da6ae302b38c57ff5b86d114951a278fb8a52e7
//META: purpose=A greater-than comparison lowers to a comparison opcode in the `cmpEQ`/`cmpLT`/... family.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's comparison-opcode row (`cmpEQ`, `cmpLT`, ...). A
// `a > b` on non-constant operands lowers to the `cmpGT` comparison
// opcode in the IR dump; `uniform` operands keep the comparison from
// being folded away.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = (a > b) ? 1 : 0;
}

// IR-LABEL: func %main
// IR: cmpGT(%{{.*}}, %{{.*}})
