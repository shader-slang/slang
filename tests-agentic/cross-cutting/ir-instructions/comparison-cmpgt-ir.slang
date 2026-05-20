//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=A greater-than comparison expression lowers to the `cmpGT` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `>` comparison between a `uniform` global and a literal lowers
// to the `cmpGT` IR opcode, which is the entry the doc cites under
// the comparison family (`cmpEQ`, `cmpLT`, ...).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = 1;
    else
        buf[0] = 2;
}

// IR-LABEL: func %main
// IR: cmpGT(%{{.*}}, 0 : Int)
