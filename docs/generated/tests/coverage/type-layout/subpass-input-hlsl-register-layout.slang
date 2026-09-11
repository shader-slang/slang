//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `SubpassInput` on the HLSL target is laid out through the D3D object rules' SubpassInput arm and receives an ordinary `t` register.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The D3D object-layout rules carry a `ShaderParameterKind::SubpassInput` arm
// returning an `InputAttachmentIndex` slot; the shipped suites only reach the
// GLSL/SPIR-V and Metal spellings of subpass inputs, never the HLSL one. This
// test compiles a plain `SubpassInput` straight to HLSL. The interesting
// observation is that the `InputAttachmentIndex` category does not replace the
// D3D binding: the variable still takes `t0`, with the Vulkan attachment index
// carried alongside as an annotation.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry fragMain -stage fragment

SubpassInput<float4> si;
float4 fragMain() : SV_Target
{
    return si.SubpassLoad();
}

//HLSL-DAG: SubpassInput<float4> si{{[A-Za-z0-9_]*}} : register(t0)
