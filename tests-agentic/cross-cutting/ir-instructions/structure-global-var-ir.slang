//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=A mutable `static` module-scope variable lowers to the documented `global_var` IR opcode (sibling of `global_param` / `globalConstant`).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `global_var` alongside `global_param` and
// `globalConstant` as the three module-scope storage opcodes. The
// existing bundle covers `global_param` (the `uniform` surface) and
// `globalConstant` (`static const`). The `global_var` opcode covers
// the mutable case: a non-`const` `static` module-scope value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static int g_counter = 0;
uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_counter = g_counter + a;
    buf[0] = g_counter;
}

// IR: global_var %gx5Fcounter
