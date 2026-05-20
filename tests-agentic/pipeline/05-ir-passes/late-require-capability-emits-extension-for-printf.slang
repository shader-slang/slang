//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=The late-require-capability pass propagates a capability requirement from `printf` to the GLSL emit's `#extension` line.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Late require capability" row of the layout-and-binding
// category (`slang-ir-late-require-capability.cpp`) propagates
// capability requirements after specialization. A `printf` call in
// GLSL emits `#extension GL_EXT_debug_printf : require`.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main -profile glsl_460

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    printf("dbg %d\n", buf[0]);
}

// GLSL: GL_EXT_debug_printf
