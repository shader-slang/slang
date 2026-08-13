//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=85f9eccdf732defa5a03b9b3664465b1a9ffc9a0344d31f04f8f61bca4e21c71
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
