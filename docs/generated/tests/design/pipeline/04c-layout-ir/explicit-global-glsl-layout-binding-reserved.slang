//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#parameter-binding-producing-the-programlayout
//META: doc_section_digest=ba2ce52ee3140a9f243e5fb99b8f2c9379dfa7db6e7567a4e73f5fca0723c7b2
//META: purpose=The third explicit-binding spelling the stage-2 row names, GLSL layout(binding=), is reserved on a global parameter before automatic allocation.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `layout(binding=)` entry in the list of annotations stage 2
// honors on global parameters. The sibling tests in this bundle cover the
// `register` and `vk::binding` spellings; this one uses the GLSL form,
// which needs `-allow-glsl`. The annotated sampler asks for binding 4 while
// the unannotated buffer is left to automatic allocation, so CHECK asserts
// the reservation was recorded first: the sampler keeps 4 and the buffer is
// auto-allocated at 0 rather than colliding.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute -allow-glsl

layout(binding = 4) uniform sampler2D gTex;

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = texture(gTex, float2(0, 0));
}

// SPV-DAG: OpDecorate %gTex Binding 4
// SPV-DAG: OpDecorate %outBuf Binding 0
