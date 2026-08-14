//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=58d6fa34e989046b10c5c18ce14f1e0e4d1c1f389944592cfe513724b2c60cae
//META: purpose=A module-scope mutable variable becomes an IR global variable that reaches emit, legalized into whatever storage form each target provides.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the global-variable branch of decl lowering. The variable is
// read back after a dispatch-bounded loop mutates it, so it cannot be
// constant-folded into its initializer or promoted to a plain local. Every
// target must therefore give it real storage, and the CHECKs pin the
// per-target legalization: HLSL keeps a `static` module global, GLSL a bare
// module global, WGSL a private-address-space variable, SPIR-V a named
// `OpVariable` of its own, and Metal / CUDA / C++ a field threaded through
// the generated kernel-context struct. The SPIR-V storage class is left
// unpinned because the legalizer picks `Private` or `Function` depending on
// how the compile is driven; what the claim needs is that the variable
// exists as storage at all.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

static int gAccum = 5;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    for (uint i = 0u; i < tid.x; ++i)
        gAccum = gAccum * 3 + int(i);
    gOut[0] = gAccum;
}

// HLSL: static int gAccum_{{[0-9]+}} = int(5);

// GLSL: int gAccum_{{[0-9]+}};

// SPIRV-DAG: OpName %gAccum "gAccum"
// SPIRV-DAG: %gAccum = OpVariable

// METAL: int gAccum_{{[0-9]+}};

// WGSL: var<private> gAccum_{{[0-9]+}} : i32;

// CUDA: int gAccum_{{[0-9]+}};

// CPP: int32_t gAccum_{{[0-9]+}};
