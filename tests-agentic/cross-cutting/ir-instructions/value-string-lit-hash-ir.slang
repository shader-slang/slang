//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=A string-literal use lowers to a `StringLit` payload-carrying value, and the module records a `global_hashed_string_literals` entry for it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `StringLit` under "Literal constants; hoistable"
// alongside `IntLit` / `FloatLit`. The user-facing surface for a
// string literal is `getStringHash("...")`, which lowers to an
// IR-level call that takes the literal as an operand and the
// compiler emits a `global_hashed_string_literals("...")` summary
// at module scope.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let h = getStringHash("hello");
    buf[0] = h;
}

// IR: getStringHash("hello")
// IR: global_hashed_string_literals("hello")
