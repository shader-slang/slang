//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T20:50:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=4310de5f7ab116db6edec98751cffebaba108c378ea42426e57d23fd6ae5cf23
//META: purpose=checkForMultiplePushConstantBlocksPerEntryPoint reports an error for an entry point that statically uses more than one push-constant block, from legalizeIRForSPIRV and from the GLSL emitter.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Check push-constant blocks row says the check fires for SPIR-V (via
// legalizeIRForSPIRV) and GLSL (via the emitter), not as a
// linkAndOptimizeIR pass. Both targets must reject the two-block entry
// point with the same diagnostic code.

//TEST:SIMPLE(filecheck=SPIRV,exit-code=-1):-target spirv -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL,exit-code=-1):-target glsl -stage compute -entry main

struct PC { float a; };
[[vk::push_constant]] ConstantBuffer<PC> pc1;
[[vk::push_constant]] ConstantBuffer<PC> pc2;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = pc1.a + pc2.a;
}

// SPIRV: error[E39033]: entry point uses more than one push-constant block
// GLSL: error[E39033]: entry point uses more than one push-constant block
